include Makefile.inc

all: $(LIBMOEPGF) $(LIBMOEPGFBENCH)
	mv $(LIBMOEPGFBENCHDIR)/$(LIBMOEPGFBENCH) .
	cp $(LIBMOEPGFDIR)/$(LIBMOEPGF) .

$(LIBMOEPGFBENCH): $(LIBMOEPG)
	cd $(LIBMOEPGFBENCHDIR); make

$(LIBMOEPGF):
	cd $(LIBMOEPGFDIR); make

.PHONY: clean
clean:
	cd $(LIBMOEPGFDIR); make clean; cd ..
	cd $(LIBMOEPGFBENCHDIR); make clean; cd ..

.PHONY: dist-clean
dist-clean: clean
	rm -fv $(LIBMOEPGFBENCH)
	rm -fv $(LIBMOEPGF)
	cd $(LIBMOEPGFDIR); make dist-clean; cd ..
	cd $(LIBMOEPGFBENCHDIR); make dist-clean; cd ..
