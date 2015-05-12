/*
 * Copyright (c) 2015 Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2015 Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This program was originally contributed by community member "Fleck" as
 * the winning entry in the December 2014 demo competition.
 *
 * Some minor changes have been made by the uGFX maintainers.
 */

#include "gfx.h"
#include "stdlib.h"
#include "string.h"
#include "orchard-math.h"
#include "orchard-app.h"

#define TETRIS_CELL_WIDTH       2
#define TETRIS_CELL_HEIGHT      2
#define TETRIS_FIELD_WIDTH      10
#define TETRIS_FIELD_HEIGHT     17
#define TETRIS_SHAPE_COUNT      7

// Size of 7-segment styled numbers
#define SEVEN_SEG_SIZE          0.6

#define SEVEN_SEG_HEIGHT            SEVEN_SEG_SIZE*3
#define SEVEN_SEG_WIDTH             SEVEN_SEG_HEIGHT*3
#define SEVEN_SEG_CHAR_WIDTH        ((SEVEN_SEG_SIZE*4)+(SEVEN_SEG_HEIGHT*2)+SEVEN_SEG_WIDTH)
#define SEVEN_SEG_CHAR_HEIGHT       ((SEVEN_SEG_SIZE*4)+(SEVEN_SEG_HEIGHT*3)+(SEVEN_SEG_WIDTH*2))
#define TETRIS_SEVEN_SEG_SCORE_X    (TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)-(strlen(pps_str)*SEVEN_SEG_CHAR_WIDTH)+(SEVEN_SEG_CHAR_WIDTH*i)

// bit0 = A
// bit1 = B
// bit2 = C
// bit3 = D
// bit4 = E
// bit5 = F
// bit6 = G
// 7segment number array, starting from 0 (array index matches number)
// 7segment number:
/*
   A
F     B
   G
E     C
   D
*/
const uint8_t   sevenSegNumbers[10]                                   = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0,1,2,3,4,5,6,7,8,9
const color_t   tetrisShapeColors[9]                                  = {Black, HTML2COLOR(0x009000), Red, Blue, Magenta, SkyBlue, Orange, HTML2COLOR(0xBBBB00), White}; // shape colors
// default tetris shapes
const int       tetrisShapes[TETRIS_SHAPE_COUNT][4][2]                = {
                                                                          {{4, 17},{4, 16},{5, 16},{4, 15}},
                                                                          {{4, 16},{5, 16},{4, 15},{5, 15}},
                                                                          {{4, 17},{5, 16},{4, 16},{5, 15}},
                                                                          {{5, 17},{5, 16},{4, 16},{4, 15}},
                                                                          {{5, 17},{5, 16},{5, 15},{4, 15}},
                                                                          {{4, 17},{4, 16},{4, 15},{5, 15}},
                                                                          {{4, 18},{4, 17},{4, 16},{4, 15}}
                                                                        };

struct tetris_context {
  // main tetris field array
  int             field[TETRIS_FIELD_HEIGHT][TETRIS_FIELD_WIDTH];
  unsigned int    game_speed;
  unsigned int    key_speed;
  systemticks_t   previous_game_time;
  systemticks_t   previous_key_time;
  int             current_shape[4][2];
  int             next_shape[4][2];
  int             old_shape[4][2];
  int             next_shape_num;
  int             old_shape_num;
  unsigned long   lines;
  unsigned long   score;

  /* Left / down / right / up / pause */
  bool_t          keys_pressed[5];
  bool_t          paused;
  bool_t          game_over;
  font_t          font16;
  font_t          font12;
};


// static void initRng(void) { //STM32 RNG hardware init
//   rccEnableAHB2(RCC_AHB2ENR_RNGEN, 0);
//   RNG->CR |= RNG_CR_RNGEN;
// }

static void initRng(void) {
  //srand(gfxSystemTicks());
}

static int uitoa(unsigned int value, char * buf, int max) {
  int n = 0;
  int i = 0;
  int tmp = 0;

  if (!buf) return -3;
  if (2 > max) return -4;
  i=1;
  tmp = value;
  if (0 > tmp) {
    tmp *= -1;
    i++;
  }
  for (;;) {
    tmp /= 10;
    if (0 >= tmp) break;
    i++;
  }
  if (i >= max) {
    buf[0] = '?';
    buf[1] = 0x0;
    return 2;
  }
  n = i;
  tmp = value;
  if (0 > tmp) {
    tmp *= -1;
  }
  buf[i--] = 0x0;
  for (;;) {
    buf[i--] = (tmp % 10) + '0';
    tmp /= 10;
    if (0 >= tmp) break;
  }
  if (-1 != i) {
    buf[i--] = '-';
  }
  return n;
}

static void sevenSegDraw(int x, int y, uint8_t number, color_t color) {
  if (number & 0x01) gdispFillArea(x+SEVEN_SEG_HEIGHT+SEVEN_SEG_SIZE, y, SEVEN_SEG_WIDTH, SEVEN_SEG_HEIGHT, color); // A
  if (number & 0x02) gdispFillArea(x+SEVEN_SEG_HEIGHT+(SEVEN_SEG_SIZE*2)+SEVEN_SEG_WIDTH, y+SEVEN_SEG_HEIGHT+SEVEN_SEG_SIZE, SEVEN_SEG_HEIGHT, SEVEN_SEG_WIDTH, color); // B
  if (number & 0x04) gdispFillArea(x+SEVEN_SEG_HEIGHT+(SEVEN_SEG_SIZE*2)+SEVEN_SEG_WIDTH, y+(SEVEN_SEG_HEIGHT*2)+SEVEN_SEG_WIDTH+(SEVEN_SEG_SIZE*3), SEVEN_SEG_HEIGHT, SEVEN_SEG_WIDTH, color); // C
  if (number & 0x08) gdispFillArea(x+SEVEN_SEG_HEIGHT+SEVEN_SEG_SIZE, y+(SEVEN_SEG_HEIGHT*2)+(SEVEN_SEG_WIDTH*2)+(SEVEN_SEG_SIZE*4), SEVEN_SEG_WIDTH, SEVEN_SEG_HEIGHT, color); // D
  if (number & 0x10) gdispFillArea(x, y+(SEVEN_SEG_HEIGHT*2)+SEVEN_SEG_WIDTH+(SEVEN_SEG_SIZE*3), SEVEN_SEG_HEIGHT, SEVEN_SEG_WIDTH, color); // E
  if (number & 0x20) gdispFillArea(x, y+SEVEN_SEG_HEIGHT+SEVEN_SEG_SIZE, SEVEN_SEG_HEIGHT, SEVEN_SEG_WIDTH, color); // F
  if (number & 0x40) gdispFillArea(x+SEVEN_SEG_HEIGHT+SEVEN_SEG_SIZE, y+SEVEN_SEG_HEIGHT+SEVEN_SEG_WIDTH+(SEVEN_SEG_SIZE*2), SEVEN_SEG_WIDTH, SEVEN_SEG_HEIGHT, color); // G
}

static void draw_shape(struct tetris_context *tetris, uint8_t color) {
  int i;
  for (i = 0; i <= 3; i++) {
    if (tetris->current_shape[i][1] <= 16 || tetris->current_shape[i][1] >= 19) {
      gdispFillArea((tetris->current_shape[i][0]*TETRIS_CELL_WIDTH)+2, gdispGetHeight()-TETRIS_CELL_HEIGHT-(tetris->current_shape[i][1]*TETRIS_CELL_HEIGHT)-3, TETRIS_CELL_WIDTH-2, TETRIS_CELL_HEIGHT-2, tetrisShapeColors[color]);
       if (color != 0) {
         gdispDrawBox((tetris->current_shape[i][0]*TETRIS_CELL_WIDTH)+2, gdispGetHeight()-TETRIS_CELL_HEIGHT-(tetris->current_shape[i][1]*TETRIS_CELL_HEIGHT)-3, TETRIS_CELL_WIDTH-1, TETRIS_CELL_HEIGHT-1, tetrisShapeColors[8]);
       } else {
         gdispDrawBox((tetris->current_shape[i][0]*TETRIS_CELL_WIDTH)+2, gdispGetHeight()-TETRIS_CELL_HEIGHT-(tetris->current_shape[i][1]*TETRIS_CELL_HEIGHT)-3, TETRIS_CELL_WIDTH-1, TETRIS_CELL_HEIGHT-1, tetrisShapeColors[0]);
       }
    }
  }
}

// static uint32_t random_int(uint32_t max) { //getting random number from STM32 hardware RNG
//   static uint32_t new_value=0;
//   while ((RNG->SR & RNG_SR_DRDY) == 0) { }
//   new_value=RNG->DR % max;
//   return new_value;
// }

static uint32_t random_int(uint32_t max) {
  return rand() % max;
}

static void create_shape(struct tetris_context *tetris) {
  int i;
  memcpy(tetris->next_shape, tetrisShapes[tetris->next_shape_num], sizeof(tetris->next_shape)); // assign from tetrisShapes arr;
  memcpy(tetris->current_shape, tetris->next_shape, sizeof(tetris->current_shape));            // tetris->current_shape = tetris->next_shape;
  memcpy(tetris->old_shape, tetris->next_shape, sizeof(tetris->old_shape));                    // tetris->old_shape = tetris->next_shape;
  for (i = 0; i <= 3; i++) {
    tetris->current_shape[i][0] += 7;
    tetris->current_shape[i][1] -= 4;
  }
  draw_shape(tetris, 0);
  tetris->old_shape_num = tetris->next_shape_num;
  tetris->next_shape_num = random_int(TETRIS_SHAPE_COUNT);
  memcpy(tetris->next_shape, tetrisShapes[tetris->next_shape_num], sizeof(tetris->next_shape)); // assign from tetrisShapes arr;
  memcpy(tetris->current_shape, tetris->next_shape, sizeof(tetris->current_shape)); // tetris->current_shape = tetris->next_shape;
  for (i = 0; i <= 3; i++) {
    tetris->current_shape[i][0] += 7;
    tetris->current_shape[i][1] -= 4;
  }
  draw_shape(tetris, tetris->next_shape_num+1);
  memcpy(tetris->current_shape, tetris->old_shape, sizeof(tetris->current_shape)); // tetris->current_shape = tetris->old_shape;
}

static void tell_score(struct tetris_context *tetris, uint8_t color) {
  char pps_str[12];
  uint8_t i;
  uitoa(tetris->lines, pps_str, sizeof(pps_str));
  gdispFillArea((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+5,
                gdispGetHeight()-50,
                gdispGetStringWidth(pps_str, tetris->font16)+4,
                gdispGetCharWidth('A', tetris->font16)+2,
                tetrisShapeColors[0]);
  gdispDrawString((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+5,
                  gdispGetHeight()-50,
                  pps_str,
                  tetris->font16,
                  tetrisShapeColors[color]);
  uitoa(tetris->score, pps_str, sizeof(pps_str));
  gdispFillArea(0,
                0,
                gdispGetWidth(),
                gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-6,
                tetrisShapeColors[0]);
  for (i = 0; i < strlen(pps_str); i++) {
    if (pps_str[i] == '0')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[0],
                   Lime);
    if (pps_str[i] == '1')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[1],
                   Lime);
    if (pps_str[i] == '2')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X, gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7, sevenSegNumbers[2], Lime);
    if (pps_str[i]
        == '3') sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X, gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7, sevenSegNumbers[3], Lime);
    if (pps_str[i] == '4')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[4],
                   Lime);
    if (pps_str[i] == '5')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[5],
                   Lime);
    if (pps_str[i] == '6')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[6],
                   Lime);
    if (pps_str[i] == '7')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[7],
                   Lime);
    if (pps_str[i] == '8')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[8],
                   Lime);
    if (pps_str[i] == '9')
      sevenSegDraw(TETRIS_SEVEN_SEG_SCORE_X,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-SEVEN_SEG_CHAR_HEIGHT-7,
                   sevenSegNumbers[9],
                   Lime);
  }
}

static void init_field(struct tetris_context *tetris) {
  int i,j;
  tell_score(tetris, 0); // clear score
  for (i = 0; i < TETRIS_FIELD_HEIGHT; i++) {
    for (j = 0; j < TETRIS_FIELD_WIDTH; j++) {
      tetris->field[i][j] = 0;
    }
  }
  create_shape(tetris);
  draw_shape(tetris, tetris->old_shape_num+1);
  tetris->lines = 0;
  tetris->score = 0;
  tell_score(tetris, 8);
}

static void drawCell(int x, int y, uint8_t color) {
  gdispFillArea((x*TETRIS_CELL_WIDTH)+2, gdispGetHeight()-TETRIS_CELL_HEIGHT-(y*TETRIS_CELL_HEIGHT)-3, TETRIS_CELL_WIDTH-2, TETRIS_CELL_HEIGHT-2, tetrisShapeColors[color]);
  if (color != 0) {
    gdispDrawBox((x*TETRIS_CELL_WIDTH)+2, gdispGetHeight()-TETRIS_CELL_HEIGHT-(y*TETRIS_CELL_HEIGHT)-3, TETRIS_CELL_WIDTH-1, TETRIS_CELL_HEIGHT-1, tetrisShapeColors[8]);
  } else {
    gdispDrawBox((x*TETRIS_CELL_WIDTH)+2, gdispGetHeight()-TETRIS_CELL_HEIGHT-(y*TETRIS_CELL_HEIGHT)-3, TETRIS_CELL_WIDTH-1, TETRIS_CELL_HEIGHT-1, tetrisShapeColors[0]);
  }
}

static void print_text(struct tetris_context *tetris, uint8_t color) {
  gdispDrawString((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+TETRIS_CELL_WIDTH,
                  gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT),
                  "Next",
                  tetris->font16,
                  tetrisShapeColors[color]);

  gdispDrawString((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+5,
                  gdispGetHeight()-70,
                  "Lines",
                  tetris->font16,
                  tetrisShapeColors[color]);
}

static void print_paused(struct tetris_context *tetris) {
  if (tetris->paused)
    gdispDrawString((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+TETRIS_CELL_WIDTH,
                     gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)/2,
                     "Paused!",
                     tetris->font12,
                     tetrisShapeColors[2]);
  else
    gdispFillArea((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+TETRIS_CELL_WIDTH,
                   gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)/2,
                   gdispGetStringWidth("Paused!", tetris->font12)+4,
                   gdispGetCharWidth('A', tetris->font12)+2,
                   tetrisShapeColors[0]);
}

static void print_game_over(struct tetris_context *tetris) {
  if (tetris->game_over)
    gdispDrawString(((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)/2)-(gdispGetStringWidth("Game Over!", tetris->font12)/2),
                    gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)/2,
                    "Game Over!",
                    tetris->font12,
                    tetrisShapeColors[2]);
  else
    gdispFillArea(((TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)/2)-(gdispGetStringWidth("Game Over!", tetris->font12)/2),
        gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)/2,
        gdispGetStringWidth("Game Over!", tetris->font12)+4,
        gdispGetCharWidth('A', tetris->font12)+2,
        tetrisShapeColors[0]);
}

static bool_t stay(struct tetris_context *tetris, bool_t down) {
  int sk, k;
  bool_t stay;
  if (down == TRUE) sk = 1; else sk = 0;
  stay = FALSE;
  for (k = 0; k <= 3; k++) {
    if (tetris->current_shape[k][1] == 0) {
      return TRUE;
    }
  }
  for (k = 0; k <= 3; k++) {
    if ((tetris->current_shape[k][0] < 0) || (tetris->current_shape[k][0] > 9)) return TRUE;
    if (tetris->current_shape[k][1] <= 16)
      if (tetris->field[tetris->current_shape[k][1]-sk][tetris->current_shape[k][0]] != 0) return TRUE;
  }
  return stay;
}

static void clear_complete_lines(struct tetris_context *tetris) {
  bool_t t;
  uint8_t reiz = 0;
  int l,k,j;
  l = 0;
  while (l <= 16) {
    t = TRUE;
    for (j = 0; j <= 9; j++)
      if (tetris->field[l][j] == 0) t = FALSE;
    if (t == TRUE) {
      for (j = 4; j >= 0; j--) { // cheap & dirty line removal animation :D
        drawCell(j,l, 0);
        drawCell(9-j,l, 0);
        gfxSleepMilliseconds(40);
      }
      reiz++;
      for (k = 0; k <= 9; k++) {
        for (j = l; j < 16; j++) {
          tetris->field[j][k] = tetris->field[j+1][k];
          drawCell(k,j, tetris->field[j][k]);
        }
      }
      for (j = 0; j <= 9; j++) {
        tetris->field[16][j] = 0;
        drawCell(j,16,0);
      }
    } else {
      l++;
    }
  }
  if (reiz > 0) {
    tetris->lines += reiz;
    tetris->score += (reiz*10)*reiz;
    tell_score(tetris, 8);
  }
}

static void go_down(struct tetris_context *tetris) {
  int i;
  if (stay(tetris, TRUE) == FALSE) {
    draw_shape(tetris, 0);
    for (i = 0; i <= 3; i++) {
      tetris->current_shape[i][1]--;
    }
    draw_shape(tetris, tetris->old_shape_num+1);
  } else {
    for (i = 0; i <= 3; i++) {
      if (tetris->current_shape[i][1] >=17) {
        tetris->game_over = TRUE;
        return;
      } else {
       tetris->field[tetris->current_shape[i][1]][tetris->current_shape[i][0]] = tetris->old_shape_num+1;
      }
    }
    clear_complete_lines(tetris);
    create_shape(tetris);
    if (stay(tetris, FALSE) == TRUE) {
      tetris->game_over = TRUE;
      return;
    }
    draw_shape(tetris, tetris->old_shape_num+1);
  }
}

static void clearField(struct tetris_context *tetris) {
  int j, k;
  for (k = 16; k >= 0; k--) {
    for (j = 0; j <= 9; j++) {
      drawCell(j,16-k, random_int(8)+1);
      gfxSleepMilliseconds(10);
    }
  }
  for (k = 0; k <= 16; k++) {
    for (j = 0; j <= 9; j++) {
      drawCell(j,16-k, tetrisShapeColors[0]);
      gfxSleepMilliseconds(10);
    }
  }
}

#define round(x) ((int)x)
static void rotate_shape(struct tetris_context *tetris) {
  int i, ox, oy, tx, ty;
  ox = tetris->current_shape[1][0];
  oy = tetris->current_shape[1][1];
  memcpy(tetris->old_shape, tetris->current_shape, sizeof(tetris->old_shape)); // tetris->old_shape = tetris->current_shape;
  for (i = 0; i <= 3; i++) {
    tx = tetris->current_shape[i][0];
    ty = tetris->current_shape[i][1];
    tetris->current_shape[i][0] = ox+(round((tx-ox)*cos(90*(3.14/180))-(ty-oy)*sin(90*(3.14/180))));
    tetris->current_shape[i][1] = oy+(round((tx-ox)*sin(90*(3.14/180))+(ty-oy)*cos(90*(3.14/180))));
  }
  if (stay(tetris, FALSE) == FALSE) {
    memcpy(tetris->next_shape, tetris->current_shape, sizeof(tetris->next_shape)); // tetris->next_shape = tetris->current_shape;
    memcpy(tetris->current_shape, tetris->old_shape, sizeof(tetris->current_shape)); // tetris->current_shape = tetris->old_shape;
    draw_shape(tetris, 0);
    memcpy(tetris->current_shape, tetris->next_shape, sizeof(tetris->current_shape)); // tetris->current_shape = tetris->next_shape;
    draw_shape(tetris, tetris->old_shape_num+1);
  } else {
    memcpy(tetris->current_shape, tetris->old_shape, sizeof(tetris->current_shape)); // tetris->current_shape = tetris->old_shape;
  }
}

static bool_t check_sides(struct tetris_context *tetris, bool_t left) {
  int sk,k;
  if (left == TRUE) sk = 1; else sk = -1;
  for (k = 0; k <= 3; k++) {
    if ((tetris->current_shape[k][0]+sk < 0) || (tetris->current_shape[k][0]+sk > 9)) return TRUE;
    if (tetris->current_shape[k][1] <= 16)
      if (tetris->field[tetris->current_shape[k][1]][tetris->current_shape[k][0]+sk] != 0) return TRUE;
  }
  return FALSE;
}

static void go_right(struct tetris_context *tetris) {
  int i;
  if (check_sides(tetris, TRUE) == FALSE) {
    draw_shape(tetris, 0);
    for (i = 0; i <= 3; i++) {
      tetris->current_shape[i][0]++;
    }
    draw_shape(tetris, tetris->old_shape_num+1);
  }
}

static void go_left(struct tetris_context *tetris) {
  int i;
  if (check_sides(tetris, FALSE) == FALSE) {
    draw_shape(tetris, 0);
    for (i = 0; i <= 3; i++) {
      tetris->current_shape[i][0]--;
    }
    draw_shape(tetris, tetris->old_shape_num+1);
  } 
}

static uint32_t tetris_init(OrchardAppContext *context) {
  return sizeof(struct tetris_context);
}

static void tetris_start(OrchardAppContext *context) {
  struct tetris_context *tetris = context->priv;

  initRng();
  tetris->next_shape_num = random_int(TETRIS_SHAPE_COUNT);
  tetris->font16 = gdispOpenFont("DejaVuSans16");
  tetris->font12 = gdispOpenFont("DejaVuSans12");

  // game key repeat speed in us
  tetris->key_speed = 140 * 1000;

  // game auto-move speed in us
  tetris->game_speed = 500 * 1000;


  // Draw the board
  gdispClear(Black);
  gdispDrawBox(0,
               gdispGetHeight()-(TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)-5,
               (TETRIS_FIELD_WIDTH*TETRIS_CELL_WIDTH)+3,
               (TETRIS_FIELD_HEIGHT*TETRIS_CELL_HEIGHT)+3,
               White);
  print_text(tetris, 8);

  init_field(tetris);
  tetris->game_over = FALSE;
  print_game_over(tetris); // removes "Game Over!" if tetrisGameOver == FALSE
  tetris->previous_game_time = gfxSystemTicks();

  orchardAppTimer(context, tetris->game_speed, true);

  gdispFlush();
}

static void tetris_exit(OrchardAppContext *context) {
  struct tetris_context *tetris = context->priv;

  clearField(tetris);
  print_game_over(tetris);

  gdispCloseFont(tetris->font16);
  gdispCloseFont(tetris->font12);
}

static void tetris_event(OrchardAppContext *context, const OrchardAppEvent *event) {
  struct tetris_context *tetris = context->priv;

  if (event->type == timerEvent) {
    go_down(tetris);
  }
  else if (event->type == keyEvent) {
    if (event->key.code == keyCW)
      rotate_shape(tetris);
    else if (event->key.code == keyCCW) {
      int i;
      for (i = 0; i < 2; i++)
        rotate_shape(tetris);
    }
    else if (event->key.code == keyLeft)
      go_left(tetris);
    else if (event->key.code == keyRight)
      go_right(tetris);
    else if (event->key.code == keySelect)
      go_down(tetris);
  }

  gdispFlush();
#if 0
static DECLARE_THREAD_FUNCTION(thdTetris, arg) {
  (void)arg;
  uint8_t i;
  while (!tetris->game_over) {
      gdispFlush();

    // key handling
    if (gfxSystemTicks() - tetrisPreviousKeyTime >= gfxMillisecondsToTicks(tetrisKeySpeed) || gfxSystemTicks() <= gfxMillisecondsToTicks(tetrisKeySpeed)) {
      for (i = 0; i < sizeof(tetris->keys_pressed); i++) {
        if (tetris->keys_pressed[i] == TRUE) {
          tetris->keys_pressed[i] = FALSE;
        }
      }
      tetrisPreviousKeyTime = gfxSystemTicks();
    }
    // auto-move part :D
    ginputGetMouseStatus(0, &ev);
    if (gfxSystemTicks() - tetrisPreviousGameTime >= gfxMillisecondsToTicks(tetris->game_speed) || gfxSystemTicks() <= gfxMillisecondsToTicks(tetris->game_speed)) {
      if ((!(ev.buttons & GINPUT_MOUSE_BTN_LEFT) || ((ev.buttons & GINPUT_MOUSE_BTN_LEFT) && !(ev.x > gdispGetWidth()/4 && ev.x <= gdispGetWidth()-(gdispGetWidth()/4) && ev.y >= gdispGetHeight()-(gdispGetHeight()/4)))) && !tetrisPaused) go_down(); // gives smooth move_down when down button pressed! :D
      tetrisPreviousGameTime = gfxSystemTicks();
    }
    if (!(ev.buttons & GINPUT_MOUSE_BTN_LEFT)) continue;
    if (ev.x <= gdispGetWidth()/4 && ev.y >= gdispGetHeight()-(gdispGetHeight()/4) && tetris->keys_pressed[0] == FALSE && !tetrisPaused) {
      go_left(tetris);
      tetris->keys_pressed[0] = TRUE;
      tetrisPreviousKeyTime = gfxSystemTicks();
    }
    if (ev.x > gdispGetWidth()-(gdispGetWidth()/4) && ev.y >= gdispGetHeight()-(gdispGetHeight()/4) && tetris->keys_pressed[2] == FALSE && !tetrisPaused) {
      go_right(tetris);
      tetris->keys_pressed[2] = TRUE;
      tetrisPreviousKeyTime = gfxSystemTicks();
    }
    if (ev.y > gdispGetHeight()/4 && ev.y < gdispGetHeight()-(gdispGetHeight()/4) && tetris->keys_pressed[3] == FALSE && !tetrisPaused) {
      rotate_shape(tetris);
      tetris->keys_pressed[3] = TRUE;
      tetrisPreviousKeyTime = gfxSystemTicks();
    }
    if (ev.x > gdispGetWidth()/4 && ev.x <= gdispGetWidth()-(gdispGetWidth()/4) && ev.y >= gdispGetHeight()-(gdispGetHeight()/4) && tetris->keys_pressed[1] == FALSE && !tetrisPaused) {
      go_down();
      tetris->keys_pressed[1] = TRUE;
      tetrisPreviousKeyTime = gfxSystemTicks();
    }
    if (ev.y <= gdispGetHeight()/4 && tetris->keys_pressed[4] == FALSE) {
      tetris->keys_pressed[4] = TRUE;
      tetrisPaused = !tetrisPaused;
      print_paused();
      tetrisPreviousKeyTime = gfxSystemTicks();
    }
  }
  return (threadreturn_t)0;
}
#endif

}

orchard_app("Tetris", tetris_init, tetris_start, tetris_event, tetris_exit);
