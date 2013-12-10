CFLAGS:= -O2 -Wall
LDFLAGS:= -lrt
SRCDIR:=src
CC:=gcc

SSE2:=$(shell cat /proc/cpuinfo | grep '^flags' | grep -o sse2 | head -n1)
SSE41:=$(shell cat /proc/cpuinfo | grep '^flags' | grep -o sse4_1 | head -n1)
AVX2:=$(shell cat /proc/cpuinfo | grep '^flags' | grep -o avx2 | head -n1)
NEON:=$(shell cat /proc/cpuinfo | grep -o neon | head -n1)

ifeq ($(NEON),neon)
CFLAGS+= -mfpu=neon -mfloat-abi=softfp
else ifeq ($(AVX2),avx2)
CFLAGS+= -mavx2
else ifeq ($(SSE41),sse4_1)
CFLAGS+= -msse4.1
else ifeq ($(SSE2),sse2)
CFLAGS+= -msse2
endif

OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*.c))
OBJS+=$(patsubst $(SRCDIR)/gf/%.c, $(SRCDIR)/gf/%.o, $(wildcard $(SRCDIR)/gf/*.c))

TARGET:=gftest

all: $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS) $(LDFLAGS)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -frv $(OBJS)

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(TARGET)

