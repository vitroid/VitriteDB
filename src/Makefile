include ../Makefile.settings
CHILD=lookupnetdb graphquench countrings2 symmetry scripts
all:
	for d in $(CHILD); do make -C $$d all; done
install:
	for d in $(CHILD); do make -C $$d install; done
clean:
	for d in $(CHILD); do make -C $$d clean; done
