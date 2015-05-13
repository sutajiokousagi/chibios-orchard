LIBFIXMATHSRC += \
                 $(LIBFIXMATH)/libfixmath/fix16.c \
                 $(LIBFIXMATH)/libfixmath/fix16_sqrt.c \
                 $(LIBFIXMATH)/libfixmath/fix16_trig.c \
                 $(LIBFIXMATH)/libfixmath/fix16_exp.c \
                 $(LIBFIXMATH)/libfixmath/fix16_str.c \
                 $(LIBFIXMATH)/libfixmath/fract32.c \
                 $(LIBFIXMATH)/libfixmath/uint32.c \

LIBFIXMATHDEFS += -DFIXMATH_FAST_SIN -DFIXMATH_NO_CACHE

LIBFIXMATHINC += $(LIBFIXMATH)/libfixmath

USE_COPT += $(LIBFIXMATHDEFS)
