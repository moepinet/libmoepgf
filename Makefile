CFLAGS:= -O2 -Wall -mavx2
LDFLAGS:= -lrt
SRCDIR:=src
CC:=gcc

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

