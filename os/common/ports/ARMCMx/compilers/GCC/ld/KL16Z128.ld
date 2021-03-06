/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * KL25Z128 memory setup.
 */

ENTRY(Reset_Handler)

MEMORY
{
  flash0   :  org = 0x00000000,  len = 0xc0
  flashcfg :  org = 0x00000400,  len = 0x10
  flash    :  org = 0x00000410,  len = 128k - 0x410 - 8k
  flashram :  org = 0x0001E000,  len = 8k
  ram      :  org = 0x1FFFF000,  len = 16k
}

__ram_start__           = ORIGIN(ram);
__ram_size__            = LENGTH(ram);
__ram_end__             = __ram_start__ + __ram_size__;

__storage_start__       = ORIGIN(flashram);
__storage_size__        = LENGTH(flashram);
__storage_end__         = __storage_start__ + __storage_size__ - 1;

SECTIONS
{
  . = 0;

  .isr : ALIGN(4) SUBALIGN(4)
  {
    KEEP(*(.vectors))
  } > flash0

  .cfmprotect : ALIGN(4) SUBALIGN(4)
  {
    KEEP(*(.cfmconfig))
  } > flashcfg

  _text = .;

  constructors : ALIGN(4) SUBALIGN(4)
  {
    PROVIDE(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE(__init_array_end = .);
  } > flash

  destructors : ALIGN(4) SUBALIGN(4)
  {
    PROVIDE(__fini_array_start = .);
    KEEP(*(.fini_array))
    KEEP(*(SORT(.fini_array.*)))
    PROVIDE(__fini_array_end = .);
  } > flash

  .text : ALIGN(4) SUBALIGN(4)
  {
    *(.text)
    *(.text.*)
    KEEP(*(SORT(.chibi_list*)));
    *(.rodata)
    *(.rodata.*)
    *(.glue_7t)
    *(.glue_7)
    *(.gcc*)
    *(.interp)
  } > flash

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > flash

  .ARM.exidx : {
    PROVIDE(__exidx_start = .);
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    PROVIDE(__exidx_end = .);
  } > flash

  .eh_frame_hdr :
  {
    *(.eh_frame_hdr)
  } > flash

  .eh_frame : ONLY_IF_RO
  {
    *(.eh_frame)
  } > flash

  .textalign : ONLY_IF_RO
  {
    . = ALIGN(8);
  } > flash

  _etext = .;
  _textdata = _etext;

  .stacks :
  {
    . = ALIGN(8);
    __main_stack_base__ = .;
    . += __main_stack_size__;
    . = ALIGN(8);
    __main_stack_end__ = .;
    __process_stack_base__ = .;
    __main_thread_stack_base__ = .;
    . += __process_stack_size__;
    . = ALIGN(8);
    __process_stack_end__ = .;
    __main_thread_stack_end__ = .;
  } > ram

  .data :
  {
    . = ALIGN(4);
    PROVIDE(_data = .);
    *(.data)
    . = ALIGN(4);
    *(.data.*)
    . = ALIGN(4);
    *(.ramtext)
    . = ALIGN(4);
    PROVIDE(_edata = .);
  } > ram AT > flash

  . = ALIGN(4);

  .bss :
  {
    . = ALIGN(4);
    PROVIDE(_bss_start = .);
    *(.bss)
    . = ALIGN(4);
    *(.bss.*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    PROVIDE(_bss_end = .);
  } > ram

  PROVIDE(end = .);
  _end            = .;

  .note.gnu.build-id :
  {
    . = ALIGN(4);
    PROVIDE( _note = .);
    *(.note.gnu.build-id)
    . = ALIGN(4);
    *(.note.gnu.build-id.*);
    . = ALIGN(4);
    PROVIDE( _enote = .);
  }  > flash

  /* ld.bfd will segfault without these */
  .rel.plt :
  {
    . = ALIGN(4);
    PROVIDE( _note = .);
  } > flash
  .got.plt :
  {
    . = ALIGN(4);
    PROVIDE( _note = .);
  } > flash
}

__heap_base__   = _end;
__heap_end__    = __ram_end__;
