include ../Makefile.inc

CFLAGS+= -I../$(LIBMOEPGFDIR)
LDFLAGS+= -lpthread

SRCDIR:=.

ifeq ($(OS), linux)
	LDFLAGS+= -lrt
endif

OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*.c))

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) ../$(LIBMOEPGFDIR)/$(LIBMOEPGF) -o $(LIBMOEPGFBENCH) $(LDFLAGS)

$(OBJS):
	$(CC) $(CFLAGS) -c $(@:.o=.c) -o $@

.PHONY: clean
clean:
	rm -fv $(OBJS)

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(GFTEST)
