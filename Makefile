CFLAGS:= -O2 -Wall
LDFLAGS:= -lrt
SRCDIR:=src
CC:=gcc

ALL_SOURCES=$(wildcard $(SRCDIR)/*.c)
SOURCES=$(filter-out %_sse2.c %_sse41.c %_avx2.c %_neon.c, $(ALL_SOURCES))

ALL_OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(ALL_SOURCES))
OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(SOURCES))

OBJS_SSE2=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_sse2.c))
OBJS_SSE41=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_sse41.c))
OBJS_AVX2=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_avx2.c))
#OBJS_NEON=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*_neon.c))

$(OBJS_SSE2):  SIMD_FLAGS_SSE2:= -msse2
$(OBJS_SSE41): SIMD_FLAGS_SSE41:= -msse4.1
$(OBJS_AVX2):  SIMD_FLAGS_AVX2:= -mavx2
#$(OBJS_NEON): SIMD_FLAGS_NEON:= -mneon

TARGET:=gftest

all: $(OBJS) $(OBJS_SSE2) $(OBJS_SSE41) $(OBJS_AVX2)
	$(CC) -o $(TARGET) $(OBJS) $(OBJS_SSE2) $(OBJS_SSE41) $(OBJS_AVX2) $(LDFLAGS)

$(OBJS_SSE2): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_SSE2) -c $(@:.o=.c) -o $@

$(OBJS_SSE41): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_SSE41) -c $(@:.o=.c) -o $@

$(OBJS_AVX2): 
	$(CC) $(CFLAGS) $(SIMD_FLAGS_AVX2) -c $(@:.o=.c) -o $@

#$(OBJS_NEON): 
#	$(CC) $(CFLAGS) $(SIMD_FLAGS_NEON) -c $(@:.o=.c) -o $@

$(OBJS):
	$(CC) $(CFLAGS) -c $(@:.o=.c) -o $@

.PHONY: clean
clean:
	rm -fv $(OBJS)
	rm -fv $(OBJS_SEE2)
	rm -fv $(OBJS_SEE41)
	rm -fv $(OBJS_AVX2)
#	rm -fv $(OBJS_NEON)

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(TARGET)

