include ../Makefile.settings
PERLCGI=vitrite.cgi
CSS=vitrite.css
FLASH=cv100.swf fcv200.swf

install: $(PERLCGI)
	install -m 0755 -o $(USER) -d $(CGIBASE)
	install -m 0755 -o $(USER) $(PERLCGI) $(CGIBASE)
	install -m 0755 -o $(USER) -d $(URLBASE)/CSS
	install -m 0644 -o $(USER) $(CSS) $(URLBASE)/CSS
	install -m 0644 -o $(USER) $(FLASH) $(URLBASE)
