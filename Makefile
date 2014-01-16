ARCH:=x86
OS:=osx

CFLAGS:= -O2 -Wall -Wextra -Isrc/
SRCDIR:=src
CC:=gcc
LDFLAGS:= 

ifeq ($(OS), linux)
LDFLAGS+= -lrt
endif

ALL_OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
OBJS=$(filter-out %_sse2.o %_sse41.o %_avx2.o %_neon.o, $(ALL_OBJS))

ifeq ($(ARCH), x86)
OBJS_SSE2=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_sse2.c))
OBJS_SSE41=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_sse41.c))
OBJS_AVX2=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_avx2.c))
else ifeq ($(ARCH), arm)
OBJS_NEON=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_neon.c))
CFLAGS+= -mfloat-abi=softfp
else
$(error Unsupported architecture $(ARCH) - please set ARCH variable in Makefile\
 to 'x86', 'arm')
endif

$(OBJS_SSE2):	SIMD_FLAGS_SSE2:= -msse2
$(OBJS_SSE41):	SIMD_FLAGS_SSE41:= -msse4.1
$(OBJS_AVX2):	SIMD_FLAGS_AVX2:= -mavx2
$(OBJS_NEON):	SIMD_FLAGS_NEON:= -mfpu=neon

ifeq ($(ARCH), x86)
OBJS_SIMD=$(OBJS_SSE2) $(OBJS_SSE41) $(OBJS_AVX2)
else ifeq ($(ARCH), arm)
OBJS_SIMD=$(OBJS_NEON)
endif

TARGET:=gftest

all: $(OBJS) $(OBJS_SIMD)
	$(CC) -o $(TARGET) $(OBJS) $(OBJS_SIMD) $(CFLAGS) $(LDFLAGS)

ifeq ($(ARCH), x86)
$(OBJS_SSE2):
	$(CC) $(CFLAGS) $(SIMD_FLAGS_SSE2) -c $(@:.o=.c) -o $@

$(OBJS_SSE41): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_SSE41) -c $(@:.o=.c) -o $@

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
	rm -fv $(TARGET)

