LIBC90TFSSRC += \
                 $(LIBC90TFS)/source/FlashInit.c \
                 $(LIBC90TFS)/source/FlashProgram.c \
                 $(LIBC90TFS)/source/FlashProgramCheck.c \
                 $(LIBC90TFS)/source/FlashProgramOnce.c \
                 $(LIBC90TFS)/source/FlashProgramSection.c \
                 $(LIBC90TFS)/source/FlashReadOnce.c \
                 $(LIBC90TFS)/source/FlashReadResource.c \
                 $(LIBC90TFS)/source/FlashVerifyBlock.c \
                 $(LIBC90TFS)/source/FlashVerifyAllBlock.c \
                 $(LIBC90TFS)/source/FlashVerifySection.c \
                 $(LIBC90TFS)/source/FlashEraseAllBlock.c \
                 $(LIBC90TFS)/source/FlashEraseBlock.c \
                 $(LIBC90TFS)/source/FlashEraseResume.c \
                 $(LIBC90TFS)/source/FlashEraseSector.c \
                 $(LIBC90TFS)/source/FlashEraseSuspend.c \
                 $(LIBC90TFS)/source/FlashGetSecurityState.c \
                 $(LIBC90TFS)/source/FlashSecurityBypass.c \
                 $(LIBC90TFS)/source/FlashCheckSum.c \
                 $(LIBC90TFS)/source/FlashCommandSequence.c \
                 $(LIBC90TFS)/source/PFlashGetProtection.c \
                 $(LIBC90TFS)/source/PFlashSetProtection.c \
                 $(LIBC90TFS)/source/CopyToRam.c \

LIBC90TFSINC += $(LIBC90TFS)/include

# change this to add configurational #defines to environment
LIBC90TFSDEFS += -DCPU_MKL25Z128VLK4
USE_COPT += $(LIBC90TFSDEFS)
