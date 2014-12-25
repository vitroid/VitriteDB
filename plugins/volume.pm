# -*- condig: utf-8; -*-
package vitrite::volume;

use strict;
use base qw(vitrite::plugin);
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);

sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["isvitrite", "ringset", "shape"];
    $self->{"inexact"} = 1;   #not useful for looking up a graph
    bless $self, $package;
}

#internal format: scalar
#$volume = '0.684266818838192';

sub name{"volume";}


sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am volume.\n";
    #if the fragment is not compact.
    if ( $record->{isvitrite} != 0 ){
	return $record->{volume} = 0;
    }

    my @coord = @{$record->{shape}{coord}};

    my @rel;
    my @sum;
    my $first = 0;
    my $m = $record->{shape}{nnode};
    foreach my $member ( 0 .. $m-1 ){
	#
	#Get the coordinates of nodes relative to the first node.
	#
	my $dx = $coord[$member][0] - $coord[$first][0];
	my $dy = $coord[$member][1] - $coord[$first][1];
	my $dz = $coord[$member][2] - $coord[$first][2];
	$rel[$member][0] = $dx;
	$rel[$member][1] = $dy;
	$rel[$member][2] = $dz;
	$sum[0] += $rel[$member][0];
	$sum[1] += $rel[$member][1];
	$sum[2] += $rel[$member][2];
    }
    #
    #Get the center-of-mass relative to the first node.
    #
    $sum[0] /= $m;
    $sum[1] /= $m;
    $sum[2] /= $m;
    #
    #Relative coordinate from the center-of-mass
    #
    foreach my $member ( 0..$m-1 ){
	$rel[$member][0] -= $sum[0];
	$rel[$member][1] -= $sum[1];
	$rel[$member][2] -= $sum[2];
    }
    #
    #determine the "volume" of the ring
    #
    my $volume = 0;
    my $nbond;
    my $length;
    foreach my $ring ( @{$record->{ringset}{ring}} ){
	my @nodes = @{$ring};
	my @out;
	my ($cx, $cy, $cz);
	foreach my $node ( @nodes ){
	    my ($xx,$yy,$zz) = @{$rel[$node]};
	    #print "$xx $yy $zz\n";
	    $cx += $xx;
	    $cy += $yy;
	    $cz += $zz;
	}
	push @nodes, $nodes[0];
	$cx /= $#nodes;
	$cy /= $#nodes;
	$cz /= $#nodes;
	for(my $i=0;$i<$#nodes;$i++){
	    my ($x1,$y1,$z1) = @{$rel[$nodes[$i]]};
	    my ($x2,$y2,$z2) = @{$rel[$nodes[$i+1]]};
	    my ($x3,$y3,$z3) = (0,0,0);
	    my ($lx,$ly,$lz) = ($x1-$x2, $y1-$y2, $z1-$z2);
	    $length += sqrt( $lx*$lx + $ly*$ly + $lz*$lz );
	    $nbond ++;
	    $x1 -= $cx;
	    $y1 -= $cy;
	    $z1 -= $cz;
	    $x2 -= $cx;
	    $y2 -= $cy;
	    $z2 -= $cz;
	    $x3 -= $cx;
	    $y3 -= $cy;
	    $z3 -= $cz;
	    my ($xx,$yy,$zz) = ( $y1*$z2-$y2*$z1,
				 $z1*$x2-$z2*$x1,
				 $x1*$y2-$x2*$y1 );
	    my $vol = ($xx*$x3 + $yy*$y3 + $zz*$z3 )/6;
	    #print "$cx $cy $cz $xx $yy $zz $vol\n";
	    $volume += abs($vol);
	}
    }
    $length /= $nbond;
    $nbond /= 2;
    #my $relvol = $volume / ($length*$length*$length);
    
    return $record->{volume} = $volume;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    if ( $record->{volume} ){
	$html .= $cgi->h2("Volume");
	$html .= $record->{volume};
    }
    $html;
}



sub to_sql{
    my ( $self, $value ) = @_;
    #$value;
    return $self->name() . "='" . ProtectSymbols($value) . "'";
}



sub from_sql{
    my ( $self, $value ) = @_;
    return "" if $value eq "";
    $value;
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    $xmlg->volume( $record->{volume} );
}



#一覧表示に使えるSQL field名
sub sortkey{
    return "volume";
}



#field名
#"*" indicates it is an approx. value.
sub keylabel{
    return "Volume*";
}



1;
