include ../Makefile.settings
DB=catalog

catalog:
	ln -s ../../CropNetwork/BCF/catalog .
install:
	install -m 0755 -o $(USER) -d $(DBPATH)
	install -m 0755 -o $(USER) $(DB) $(DBPATH)
