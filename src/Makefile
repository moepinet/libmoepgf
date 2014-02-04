include ../Makefile.inc

CFLAGS+= -I../$(LIBGFDIR)

SRCDIR:=.

ifeq ($(OS), linux)
	LDFLAGS+= -lrt
endif

OBJS=$(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(wildcard $(SRCDIR)/*.c))

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) ../$(LIBGFDIR)/$(LIBGF) -o $(GFTEST)

$(OBJS):
	$(CC) $(CFLAGS) -c $(@:.o=.c) -o $@

.PHONY: clean
clean:
	rm -fv $(OBJS)

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(GFTEST)
