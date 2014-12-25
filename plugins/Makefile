include ../Makefile.settings
PERLLIB=ringcount.pm ringset.pm eulerindex.pm shape.pm graph.pm crystalviewer.pm topovolume.pm volume.pm image.pm plugin.pm isvitrite.pm symmetry.pm spectrum.pm rd.pm solid.pm

install: $(PERLCGI)
	install -m 0755 -o $(USER) -d $(PERLLIBPATH)
	install -m 0755 -o $(USER) $(PERLLIB) $(PERLLIBPATH)
