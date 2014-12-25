include Makefile.settings

all:
	for d in plugins lib cgi-bin src; do make -C $$d all; done
install:
	for d in plugins lib cgi-bin src; do make -C $$d install; done
install-db:
	for d in DB; do make -C $$d install; done
