include ../Makefile.inc

SRCDIR:=.
AR:=ar
ARFLAGS:=rcs

ALL_OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
OBJS=$(filter-out %_sse2.o %_ssse3.o %_avx2.o %_neon.o, $(ALL_OBJS))

ifeq ($(ARCH), x86)
OBJS_SSE2=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_sse2.c))
OBJS_SSSE3=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_ssse3.c))
OBJS_AVX2=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_avx2.c))
else ifeq ($(ARCH), arm)
OBJS_NEON=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_neon.c))
CFLAGS+= -mfloat-abi=softfp
else
$(error Unsupported architecture $(ARCH) - please set ARCH variable in Makefile\
 to 'x86', 'arm')
endif

$(OBJS_SSE2):	SIMD_FLAGS_SSE2:= -msse2
$(OBJS_SSSE3):	SIMD_FLAGS_SSSE3:= -mssse3
$(OBJS_AVX2):	SIMD_FLAGS_AVX2:= -mavx2
$(OBJS_NEON):	SIMD_FLAGS_NEON:= -mfpu=neon

ifeq ($(ARCH), x86)
OBJS_SIMD=$(OBJS_SSE2) $(OBJS_SSSE3) $(OBJS_AVX2)
else ifeq ($(ARCH), arm)
OBJS_SIMD=$(OBJS_NEON)
endif

all: $(OBJS) $(OBJS_SIMD)
	$(AR) $(ARFLAGS) $(LIBGF) $(OBJS) $(OBJS_SIMD)

ifeq ($(ARCH), x86)
$(OBJS_SSE2):
	$(CC) $(CFLAGS) $(SIMD_FLAGS_SSE2) -c $(@:.o=.c) -o $@

$(OBJS_SSSE3): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_SSSE3) -c $(@:.o=.c) -o $@

$(OBJS_AVX2): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_AVX2) -c $(@:.o=.c) -o $@

else ifeq ($(ARCH), arm)
$(OBJS_NEON): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_NEON) -c $(@:.o=.c) -o $@
endif

$(OBJS):
	$(CC) $(CFLAGS) -c $(@:.o=.c) -o $@

.PHONY: clean
clean:
	rm -fv $(OBJS) $(OBJS_SIMD)

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(LIBGF)

