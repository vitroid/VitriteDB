include ../Makefile.settings
BINTARGET=graphquench2 graphquench4c graphquench 
TARGET=$(BINTARGET) rngs2ar3a.pl totalgraphquench.sh #totalgraphquench2.sh 

all: $(BINTARGET)
graphquench: graphquench.o frprmn.o linmin.o f1dim.o brent.o mnbrak.o nrutil.o Graph.o
	$(CC) $^ -o $@ $(LOCALLDFLAGS) $(LDFLAGS) -lm
graphquench2: graphquench2.o frprmn.o linmin.o f1dim.o brent.o mnbrak.o nrutil.o Graph.o
	$(CC) $^ -o $@ $(LOCALLDFLAGS) $(LDFLAGS) -lm
graphquench4c: graphquench4c.o frprmn.o linmin.o f1dim.o brent.o mnbrak.o nrutil.o Graph.o
	$(CC) $^ -o $@ $(LOCALLDFLAGS) $(LDFLAGS) -lm
