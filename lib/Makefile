include ../Makefile.settings
PERLLIB=settings.pm attrdb.pm make.pm html.pm search.pm

install: $(PERLCGI)
	install -m 0755 -o $(USER) -d $(PERLLIBPATH)
	install -m 0755 -o $(USER) $(PERLLIB) $(PERLLIBPATH)
