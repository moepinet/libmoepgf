include Makefile.inc

all: $(LIBGF) $(BENCH)
	cp $(BENCHDIR)/$(BENCH) .
	cp $(LIBGFDIR)/$(LIBGF) .

$(BENCH): $(LIBGF)
	cd $(BENCHDIR); make

$(LIBGF):
	cd $(LIBGFDIR); make

.PHONY: clean
clean:
	cd $(LIBGFDIR); make clean; cd ..
	cd $(BENCHDIR); make clean; cd ..

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(BENCH)
	rm -fv $(LIBGF)
	cd $(LIBGFDIR); make dist-clean; cd ..
	cd $(BENCHDIR); make dist-clean; cd ..
