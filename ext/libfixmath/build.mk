LIBFIXMATHSRC += \
                 $(LIBFIXMATH)/libfixmath/fix16.c \
                 $(LIBFIXMATH)/libfixmath/fix16_sqrt.c \
                 $(LIBFIXMATH)/libfixmath/fix16_trig.c \
                 $(LIBFIXMATH)/libfixmath/fix16_exp.c \
                 $(LIBFIXMATH)/libfixmath/fix16_str.c \
                 $(LIBFIXMATH)/libfixmath/fract32.c \
                 $(LIBFIXMATH)/libfixmath/uint32.c \
                 $(LIBFIXMATH)/contrib/fix16_fft.c \

LIBFIXMATHDEFS += -DFIXMATH_FAST_SIN -DFIXMATH_NO_CACHE

LIBFIXMATHINC += $(LIBFIXMATH)/libfixmath $(LIBFIXMATH)/contrib

USE_COPT += $(LIBFIXMATHDEFS)
