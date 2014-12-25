#!/usr/bin/env perl
#
#rewritten from polyhed.pl
#
#it projects a vitrite on a flat plane.
#
#

use strict;

#
#Global vars
#

#
#$Limit specifies the largest size of the rings utilized for fragments.
#
my $MAXRINGSIZE;
my $MAXFRAGSIZE;
my $DEBUG = 1;
my $YAPLOT=0;
while( 0 <= $#ARGV ){
    if ( $ARGV[0] eq "-x" ){
	shift;
	$MAXRINGSIZE = $ARGV[0];
    }
    if ( $ARGV[0] eq "-l" ){
	shift;
	$MAXFRAGSIZE = $ARGV[0];
    }
    elsif ( $ARGV[0] eq "-d" ){
	$DEBUG ++;
    }
    elsif ( $ARGV[0] eq "-y" ){
	$YAPLOT ++;
    }
    shift;
}

my @PLACED;
my @RINGSINTHEPOLY;
my @NUMRINGSATTHENODE;
my $NPOLY=0;
my $NUMNODES;
my %EDGES;
my $ORIGINALCOMPO;

my @coord;
my $pi2 = 3.14159*2;

#
#list rings having the specified 3 successive nodes (center, left, and right)
#
sub attach{
    my ( $nodes, $center, $left, $right, $perimeter ) = @_;

    my $nn = $#{$nodes};
    my @n = @{$nodes};
    #print join(" ", "RING:", @n),"\n";
    #
    #fail safe
    #
    push @n, $n[0];
    my $i;
    for($i=0; $i<= $nn; $i++ ){
	last if ( $n[$i] == $center );
    }
    if ( $n[$i+1] != $right ){
	@n = reverse @{$nodes};
	$i = $nn - $i;
    }
    #
    #rotate and move center to the first element of @n
    #
    @n = (@n[0..$nn],@n[0..$nn])[$i..$i+$nn];
    #print join(" ", "RING>", @n),"\n";

    my @p = @{$perimeter};
    #print join(" ", "PERI:", @p),"\n";
    for($i=0;$i<=$#p;$i++){
	last if $p[$i] == $center;
    }
    @p = (@p,@p)[$i..$i+$#p];
    #print join(" ", "PERI>", @p),"\n";

    #
    #match reversely
    #
    @p = reverse @p;
    @n = reverse @n;
    for( $i=0;$i<=$#n;$i++ ){
	last if $n[$i] != $p[$i];
    }
    @p = reverse( (@p,@p)[$i..$i+$#p] );
    @n = reverse( (@n,@n)[$i..$i+$#n] );

    my @shared;
    while( $n[0] == $p[0] && 0 <= $#n){
	push @shared, shift @n;
	shift @p;
    }
    #print join(" ", "PERI>", @p),"\n";
    my @newperi;
    if ( 0 <= $#p ){
	@newperi =  ( $shared[0], reverse(@n), $shared[$#shared], @p );
	#
	#determine the edge length
	#
	my ($x1,$y1) = @{$coord[$shared[0]]};
	my ($x2,$y2) = @{$coord[$shared[$#shared]]};
	my ($dx,$dy) = ($x1-$x2, $y1-$y2);
	my $elen = sqrt( $dx*$dx + $dy*$dy );
	#
	#determine the number of nodes on the bulge
	#
	my $nnode = $#n+3;
	#print $nnode, "\n";
	#
	#go turtle
	#
	my $theta = -$pi2 / $nnode;
	my $sinth = sin($theta);
	my $costh = cos($theta);
	my ( $ex, $ey ) = ($dx/$elen, $dy/$elen);
	#rotate
	for(my $i=$#n; $i>=0; $i--){
	    ( $ex, $ey ) = ( $ex * $costh + $ey * $sinth, -$ex *$sinth+$ey*$costh);
	    $x1 += $ex * $elen;
	    $y1 += $ey * $elen;
	    $coord[$n[$i]] = [ $x1, $y1 ];
	    #print "t $x1 $y1 0 " . $n[$i] . "\n";
	}
    }
    #print join(" ", "NEWP:", @newperi),"\n";
    return @newperi;
}


my $ringset = LoadRNGS();


$NUMNODES = $ringset->{size};
#
#Node list of all rings
#
my @rings;
#
#All node triplets
#
my %node3;
#
#All HB pairs
#
undef %EDGES;

foreach my $ring ( @{$ringset->{ring}} ){
    my @nodes = @{$ring};
    my $size = $#nodes + 1;
    #
    #register to ring list
    #
    push @rings, [ @nodes ];
    #
    #register the ring to "node triplets" owner list 
    #
    push @nodes,$nodes[0], $nodes[1];
    for(my $i=0; $i<$size; $i++){
	#
	#Register node triplets.
	#first key is the central node. not the first node of 3 nodes.
	#
	push @{ $node3{$nodes[$i+1]}{$nodes[$i]}{$nodes[$i+2]} }, $#rings;
	push @{ $node3{$nodes[$i+1]}{$nodes[$i+2]}{$nodes[$i]} }, $#rings;
	#
	#Register bonds
	#
	$EDGES{$nodes[$i]}{$nodes[$i+1]} = 1;
	$EDGES{$nodes[$i+1]}{$nodes[$i]} = 1;
    }
}
foreach my $ring ( 0 .. $#rings ){
    LookupPolyhed( \@rings, \%node3, $ring );
}
exit 0;


sub LookupPolyhed{
    my ( $ringlist, $node3, $originring ) = @_;

    #
    #perimeter of the current fragment
    #
    my @perimeter = @{ $ringlist->[$originring] };
    #
    #already placed rings
    #
    $PLACED[$originring] = 1;
    push @RINGSINTHEPOLY, $originring;
    #
    #set coordinates
    #
    my $nnode = $#perimeter+1;
    for(my $i=0; $i<$nnode;$i++){
	$coord[$perimeter[$i]] = [ cos( $i*$pi2 / $nnode ), 
				   sin( $i*$pi2 / $nnode ) ];
    }
    #
    #share number of each node
    #
    foreach my $node ( @perimeter ){
	$NUMRINGSATTHENODE[$node]++;
    }
    #
    #you can start wherever you want.
    #
    my $center = $perimeter[0];
    my $right = $perimeter[1];  #right must be the succ element of center

    foreach my $left ( keys %{ $node3->{$center}{$right} } ){
	ExtendPerimeter( $ringlist, $node3, $originring, \@perimeter, $left, $center, $right );
    }

    $PLACED[$originring] = 0;
    pop @RINGSINTHEPOLY;
    #
    #share number of each node
    #
    foreach my $node ( @perimeter ){
	$NUMRINGSATTHENODE[$node]--;
    }
}



#
#Extend the perimeter of adjacent rings by attaching a new ring on the
# perimeter.
#
sub ExtendPerimeter{
    my ( $ringlist, $node3, $originring, $peri, $left, $center, $right ) = @_;
    my @perimeter = @{$peri};

    #
    #Limit the fragment size.
    #
    if ( $MAXFRAGSIZE && $MAXFRAGSIZE <= $#RINGSINTHEPOLY + 1 ){
	#print STDERR "-";
	return;
    }

    #print "Extension.\n";
    #
    #for each ring owning the triplets.
    #
    foreach my $ring ( @{ $node3->{$center}{$right}{$left} } ){
	#print "$left-$center-$right $originring, $ring\n";
	#
	# origin ring must have the smallest label.
	#
	if ( $originring < $ring ){
	    if ( ! $PLACED[$ring] ){
		my @newperi = attach( $ringlist->[$ring], $center, $left, $right, \@perimeter );
		#
		#mark the ring as used.
		#
		$PLACED[$ring] = 1;
		push @RINGSINTHEPOLY, $ring;
		#
		#Increment the number of rings sharing the node.
		#
		foreach my $node ( @{$ringlist->[$ring]} ){
		    $NUMRINGSATTHENODE[$node] ++;
		}
		#
		#If perimeter vanishes, i.e. if the polyhedron closes,
		#
		if ( $#newperi == -1 ){
		    #
		    #filter by complexity
		    #
		    my %bonds;
		    my %newlabel;
		    my $nlabel = 0;
		    my $nbond =0;

		    foreach my $ring ( @RINGSINTHEPOLY ){
			my @nodes = @{$ringlist->[$ring]};
			push @nodes, $nodes[0];
			for(my $j=0;$j<$#nodes; $j++){
			    my $x = $nodes[$j];
			    if ( ! defined $newlabel{$x} ){
				$newlabel{$x} = $nlabel ++;
			    }
			    my $y = $nodes[$j+1];
			    if ( ! defined $newlabel{$y} ){
				$newlabel{$y} = $nlabel ++;
			    }
			    #$x = $newlabel{$x};
			    #$y = $newlabel{$y};
			    if ( ! defined $bonds{$x}{$y} ){
				$nbond++;
			    }
			    $bonds{$x}{$y} = 1;
			    $bonds{$y}{$x} = 1;
			}
		    }
		    my $nring = $#RINGSINTHEPOLY + 1;
		    #print STDERR "($nring)";
		    #print "$nring - $nbond + $nlabel - 1 \n" if $DEBUG;
		    #print "Complexity = ", $nring - $nbond + $nlabel - 1, "\n" if $DEBUG;
		    #if ( $nring - $nbond + $nlabel - 1 == 1){
		    print "\@NGPH\n$nlabel\n" unless $YAPLOT;
		    foreach my $i ( keys %bonds ){
			foreach my $j ( keys %{$bonds{$i}} ){
			    if ( $i < $j ){
				if ( $YAPLOT ){
				    my ($xi,$yi) = @{$coord[$i]};
				    my $zi = ($xi*$xi + $yi*$yi)/10;
				    my ($xj,$yj) = @{$coord[$j]};
				    my $zj = ($xj*$xj + $yj*$yj)/10;
				    print "l $xi $yi $zi $xj $yj $zj\n";
				}
				else{
				    print "$i $j\n";
				}
			    }
			}
		    }
		    print "-1- 1\n" unless $YAPLOT;
		    print "\@AR3A\n$nlabel\n" unless $YAPLOT;
		    for(my $i=0; $i<$nlabel; $i++){
			my ($x,$y) = @{$coord[$i]};
			my $z = ($x*$x + $y*$y)/10;
			if ( $YAPLOT ){
			    print "t $x $y $z $i\n";
			}
			else{
			    print "$x $y $z\n";
			}
		    }
		    $NPOLY ++;
		    #}
		}
		else{
		    #
		    # perimeter still exists.
		    #
		    my $error = 0;
		    my $newcenter   = -1;
		    for(my $i=0; $i<=$#newperi; $i++){
			my $node = $newperi[$i];
			if ( 2 < $NUMRINGSATTHENODE[$node] ){
			    $error = 1;
			    last;
			}
			if ( 2 == $NUMRINGSATTHENODE[$node] ){
			    $newcenter = $i;
			}
		    }
		    if ( ! $error ){
			if ( 0 <= $newcenter ){
			    my $newleft = $newcenter - 1;
			    my $newright = $newcenter + 1;
			    if ( $newleft < 0 ){
				$newleft = $#newperi;
			    }
			    if ( $#newperi < $newright ){
				$newright = 0;
			    }
			    ExtendPerimeter( $ringlist, $node3, $originring, \@newperi, $newperi[$newleft], $newperi[$newcenter], $newperi[$newright] );
			}
			else{
			    die "Thin perimeter.\n";
			}
		    }
		}
		#
		#recover perimeter by detaching the ring
		#
		foreach my $node ( @{$ringlist->[$ring]} ){
		    $NUMRINGSATTHENODE[$node] --;
		}
		$PLACED[$ring] = 0;
		pop @RINGSINTHEPOLY;
	    }
	}
    }
}



sub LoadRNGS{
    while(<STDIN>){
	if ( /^\@RNGS/ ){
	    my $ringset;
	    my $nnode = <STDIN>;
	    $ringset->{size} = $nnode + 0;
	    while(<STDIN>){
		chomp;
		my @nodes = split;
		last if $nodes[0] == 0;
		shift @nodes;
		push @{$ringset->{ring}}, [ @nodes ];
	    }
	    return $ringset;
	}
    }
}

