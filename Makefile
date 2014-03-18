include Makefile.inc

all: libgf gftest
	mv $(GFTESTDIR)/$(GFTEST) .
	cp $(LIBGFDIR)/$(LIBGF) .

gftest: libgf
	cd $(GFTESTDIR); make

libgf:
	cd $(LIBGFDIR); make

.PHONY: clean
clean:
	cd $(LIBGFDIR); make clean; cd ..
	cd $(GFTESTDIR); make clean; cd ..

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(GFTEST)
	rm -fv $(LIBGF)
	cd $(LIBGFDIR); make dist-clean; cd ..
	cd $(GFTESTDIR); make dist-clean; cd ..
