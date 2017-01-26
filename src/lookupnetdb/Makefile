include ../Makefile.settings
BINTARGET=lookupnetdb spectrum
#BINTARGET=spectrum
TARGET=$(BINTARGET)

all: $(BINTARGET)
lookupnetdb: GraphDB2.o dblookup.o dm.o Isomorph.o NetMatch.o Mark2.o SimpleGraph.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
spectrum: spectrum.o dm.o Isomorph.o NetMatch.o Mark2.o SimpleGraph.o
	$(CC) $(CFLAGS) $^ -o $@  -L/usr/local/opt/openssl/lib/ -lcrypto # $(LDFLAGS)
