# Schema for light genes.

A light genome is nominally a diploid genome consisting of a set of characteristics that can be
"bred" among each other.

---

# Some base rules of the genome properties

* All colors are in HSV
* The phenotype of each gene is safe in its own right: you can't have a non-sense gene,
or a gene that crashes the system.
* Combinations of phenotypes can happen that don't necessarily make sense or produce an
undersireable effect.
* A set of gene is called an "indivdiual". All individuals are hermaphrodites, e.g.
can function as a role of male or female.

Think of genes as more like a set of sliders to effect modules, rather than as code
for effects themselves.

These are examples of logical genes:

* Periodicity of a sine wave
* Color gamut
* Accelerometer fall sensitivity threshold
* Accelerometer action (e.g, triggers strobe, dims, reverses pattern)
* Sound sensitivity threshold
* Sound action (e.g. what behavior happens in response to sound)
* Strobing frequency (if such a phenotype was to be displayed)

These are examples of genes that have no logic on their own right:

- Loop counters
- Snippets of executable code
- Entry points into existing code

In other words, genes set parameters on existing, well-vetted functions that can make a lighting effect in its own right, and are not raw snippets of code that we hope can run and not crash the embedded OS. The main motivation for this restriction is to limit the amount of testing required to ensure that the system is stable in the context of a diverse set of user-settable parameters.

---
#Lighting Genes and Inheritance

A lighting effect genome consists of several diploid chromosomes, each
consist of several alleles.

Each "chromosome" is responsible for an effect group, such as:

* Coloration
* Cycling dimming
* Linear patterns
* Strobing
* Accelerometer behavior
* Microphone behavior
* Touch surface behaivor

Alleles within a chromosome are combined together from each of the
haploid copies to create a net effect. Dominance or recessiveness of a
particular allele isn't an explicit bit in the allele; it is a result
from the method of applying the alleles to the algorithm, and the
value of the allele itself.

For example, a brightness allele may be encoded as an integer from
0-255. A genome has two copies of this allele, and the application of
the allele may be (again only for sake of example) the saturating
addition of the two alleles. In this case, a "dim" allele is
recessive, as the presence of a "dim" and a "bright" value causes the
net result to be bright (due to the nature of a saturating
add). However, one could also choose to make "dim" a dominant gene by
encoding the value as a binary fraction and multiplying the two
together. In this case, the presence of a "dim" allele will result in
a dim phenotype due to the nature of the multiplication combination
function.

A result of this method of using the combination function to control
dominance and recessivenes is that all "mutations" of a particular
allele are still "useful" and it also allows a gene pool with only
"dominant" behavior to result in offspring with "recessive" behavior
in the case of a double-mutation on the allele that results in
recessiveness.

---
#Defining Genes

Every gene definition consists of:

* A parameter (defines what the gene does)
* An encoding of the parameter (typically a one-byte number, and the
  encoding maps the value of the byte to the effect of the parameter)
* An expression function (defines how pairs of alleles are combined
  into the final parameter value)

Note: all integers are encoded to gray code prior to the application
of the mutation function, and then decoded back into binary
numbers. This allows mutation rates to be higher without risk of
falling off a functional cliff on the genetic pool.

Functions used here:
* satadd(A, B, sat) returns A + B > sat ? sat : A + B;
* map(A,B, C,D) maps integers from range A:B onto range C:D

##Cyclic dimming chromosome (cd)
Affects the V value in an HSV color.

###Periodicity gene (cd-period)

* Parameter: defines period of the sine wave function overlaid on a
  lighting effect e.g. sin(Periodicity*jwt + phase)
* Encoding: Integer from 0-6 than is inversely proportional to the
  number of sine periods within the length of the lighting chain.
* Expression function: The dominant behavior should tend towards a
  periodicity of 0. As a result, the function is (6 - satadd(A,B,6)).

###Rate gene (cd-rate)

* Parameter: defines the rate at which the wave function precesses
  over time (the phase portion of the sin(jwt + phase) function)
* Encoding: Integer from 0-255 which maps to a range from 250-8000. The final
  range parameter is the time in milliseconds for a period to complete (from 4Hz to 0.25Hz)
* Expression function: The dominant behavior should tend toward the mean. Thus,
  the expression function is avg(A,B)
  
###Direction gene (cd-dir)
* Paraemeter: defines the direction of the wave function precession
* Encoding: Integer from 0-255. Values above 128 set the sign to CW.
* Expression function: satadd(A,B). CW is dominant; CCW is recessive.

##Saturation chromosome (sat)

Affects the S value. Has only one gene (sat)

* Parameter: defines the saturation value
* Encoding: Integer from 0-255
* Expresion function: satadd(A,B) -- saturated colors tend to be dominant

##Hue chromosome (hue)

Affects the H value. 

###Hue rate and direction (hue-ratedir)
* Parameter: defines the rate at which the hue color precesses
* Encoding: bits[3:0] encode a rate, from 0-15. bits[7:4] encode a direction.
* Expression function:
    - The dominant behavior should be toward a static hue motion, and so rate is computed as (9 - satadd(A, B, 9)).
    - The dominant behavior should be toward a CCW rotation. So, satadd(A,B,15), with values above 7 setting the sign to CCW.

###Hue gamut base (hue-base)
* Parameter: defines the starting point for the hue cycle
* Encoding: integer 0-255
* Expression function: Dominant behavior should be toward a low base. Thus, satsub(A,B,0). In other words, negative numbers snap to 0, leading to a 0 base half the time.

###Hue gamut bounds (hue-bound)
* Parameter: defines the maximum point for the hue cycle
* Encoding: integer 0-255
* Expression function: Dominant behavior should be toward a high bound. Thus, 255-satsub(A,B,0).

The combination of base and bounds defines the hue cycle. Around the 16 LEDs, 0-7 map from base to bounds, and then 8-15 map from bounds to base. This bi-cyclic mapping then has the rate and direction parameter added onto it.  

Question: should everyone get a full gamut, just part of it is shown at all times, or should the range of the gamut be limited at all times? The concern is that if only a partial gamut is shown, the genetic pool will start to gravitate toward a single color, and everyone will look the same. So I think the behavior should be cycling through the whole gamut, but the amount of the gamut that you see at any point in the cycle is defined by the base/bounds behavoir.

On the other hand, I could see people trying to "breed" for their favorite color. If the whole gamut is always shown, it prevents people from color-picking. This argues for the base/bounds function to be applied after the rate and direction, so you're cycling only within the limited range of the base and bounds all the time.

##Linear chromosome (lin)

This chromosome will specify the overlay of a linear effect pattern ("rain drop" or "shooting" pattern) over the color cycling pattern. 

This should be a rare variant. When it is expressed, the rate of linear shooting, the color & saturation should be encodable. Extra linear droplets will always be injected in response to accelerometer stimulus. 

##Strobe chromosome (strobe)

This chromosome will specify if a strobing behavior is expressed. The strobing behavior isn't expressed regularly (that would be _so annoying_), but it can come out in response to certain stimuli, e.g. accelerometer hits, music, or touchsurface stimulation.


##Accelerometer chromosome (accel)

Defines the sensitivity of the accelerometer. Should tend toward a "sane" parameter.

##Microphone chromosome (mic)

What should the microphone do? The microphone is tricky beause it's computationally expensive to do a reasonable frequency extraction; and a simple volume-based threshold will trigger all the time. 

Maybe one parameter to tweak is to have the saturation and/or value modulated based upon sound intensity. So, if you're in a quiet place, the badge will appear washed out or dark; but if you go to a party area, it will become brighter and more saturated. 

##Touch surface chromosome (touch)

What does the touch surface do? I think in general, there should be a default behavior across all devices, regardless of the chromosome -- the touch surface is a stimulus that someone can use to modify the behavior, so it's environmental and not genetic. 

* The jog dial, if spun CW, will increase saturation. The function "bumps" the saturation value up by 2 every spin. The bump will decerease to baseline with one tick per second. 
* The jog dial, if spun CCW, will increase the rate of the cyclic dimming, using the same bump feature as the saturation. 
* The right button will cycle through indiviuals
* The left button will cycle dimming state
* Left + right escapes you to the main menu

The center button is what will have a breedable behavior. 

### Button behavior (touch-fx)
* Parameter sets the middle button behavior
* Encoding: integer 0-255
* Expression function: value % (number of behaviors) resulting in a behavior. In other words, the behavior is random. The supported behaviors are:
    * Do nothing
    * Strobe for 1 second
    * Shoot linear pattern for 1 second

---

## Mutations
Every time a child is born, there is a chance of introucing novel mutations. The mutation
rate is set by a global parameter, and is typically low but non-zero (e.g. about a 50% chance
probability on every breeding run that you'll have a mutation with hamming distance 1 one
the entire set of parameters).

Mutation rate can be increased by vigorously shaking the device prior to having sex. There
is a completion bar on the UI to indicate relative mutation rate, and the more the user
shakes, the higher the bar gets. Over time, the bar will start to empty as well. In its
maximal setting, the mutation rate should be 100% chance of mutation of about half the genes
with a hamming distance > 4.

##Breeding effects
Users are given a set of 5 individuals. They can mark any of the 5 individuals as "locked", which are preserved through breeding rounds. Non-locked individuals are replaced with new children every breeding round. Any individual (including non-locked) may be selected as the effect that is being rendered on the display at any given time.

Any individual used for breeding is automatically marked as locked, and is not replaced by the breeding process. 

In the code, probably the # of individuals should be set up as a #define or otherwise easily modified parameter, as it's unclear as to what the correct breeding pool is vs overwhelming someone with too many choices to sift through in the course of a breeding program. 

The feeling is two is too small for genetic diversity; eight is too many to select among, you'll spend all your time staring at effects trying to pick the best one. So four seems to be a good compromise between diversity and tractability of selection.

##The mechanics of having sex
The set of individual light genomes are replayed and controlled by one app. 

From the LED viewing state:

* Make sure that the individual you want to breed with is the one currently showing.
* Exit to the main menu
    * Select "have sex" 
* A dialog box pops up with a warning, indicating which individuals will be replaced by having sex (playtime should be indicated as well). "Warning: these individuals will be replaced!" If the user picks OK, proceed to next menu.
* A menu shows up with mates nearby, based on ping scan feature. 
    * Pick the mate you want to have sex with.
* The mate will receive a pop-up asking if they consent to sex
* If yes, the genome of the currently displayed pattern on the mate will be transmitted to the initiator.
* The initiator now has the genome of the mate, and the RF protocol is done.


* Note that the initiator is always the mother, and is the only one who can have children.
* A completion bar pops up with a count-down timer and the message "mutation rate". Shaking the badge vigorously will fill up the completion bar.
* The genetic cross-over is computed based upon the mutation rate.
* The new children are automatically populated over the non-locked, non-currently displayed pattern.

Sex is now complete.

##Family Lineage
Every genome will have associated with it a list in FLASH which contains the last 100 sexual encounters, so everyone can view the family lineage of a particular individual.

This might be a pain in the ass to implement, so this will be last on the list of features to create.

---
#List of coding tasks based on feature spec

* A generalized scrollable list widget? To implement mate picking, individual picking, etc.
* A general widget for entering names from an alphabetic character set
* A widget for managing genomic data -- reading, writing, updating the FTL
* Radio ping protocol
* Radio paging protocol
* Sexual data transfer protocol (happens at a lower power than radio ping/paging)
* The actual breeding algorithm itself (randomization, cross-over of diploid genes, etc.)
* The actual lighting effect computation based on the diploid genome
* Accelerometer-based completion bar screen

Other features on the list:

* Set your name
* Force charging mode
* Force boost mode
* Enable external charging port (with auto-dimming of LEDs)
* Set party timer
    * Auto-dimming at <40% battery if party timer won't be met
* Battery status (mAh remaining, current mAh consumption, voltage)
* Automatic detection/rollover of charging/boost mode (with D+/D- ECO)
* Sleep mode (low power shutdown mode, but not full power off; wake with captouch)
* Improve shipmode so NV data is flushed before shipmode engaged
