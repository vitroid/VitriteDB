# use gmake
include ../Makefile.settings
BINTARGET	=symmetry
CDEBUGFLAGS	=#-g -Wall -Werror
#CFLAGS=-DMAP=0,1,2,3,4,5,6,7,8,9,10,11,12 -g
CFLAGS=-DMAP=2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 -g
TARGET=$(BINTARGET)
#include $(HOME)/etc/Makefile
all: $(BINTARGET)
symmetry: symmetry.o Mark2.o dm.o
	$(CC) $(CFLAGS) $^ -o $@ $(LOCALLDFLAGS) $(LDFLAGS)
