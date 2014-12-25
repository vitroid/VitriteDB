# -*- condig: utf-8; -*-
package vitrite::solid;

use strict;
use Math::Trig;
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
#$solid = '0.999999999999999';

sub name{"solid";}




my $pi = 3.14159265358979323846264338327950;

sub normalize{
  my ( $v ) = @_;
  my $siz = sqrt( $v->[0]*$v->[0] + $v->[1]*$v->[1] + $v->[2]*$v->[2]);
  return ( $v->[0]/$siz, $v->[1]/$siz, $v->[2]/$siz );
}              

sub innerprod{
  my ( $v1,$v2 ) = @_;
  return $v1->[0]*$v2->[0] + $v1->[1]*$v2->[1] + $v1->[2]*$v2->[2];
}

sub outerprod{
  my ( $v1,$v2 ) = @_;
  my $x = $v1->[1]*$v2->[2] - $v1->[2]*$v2->[1];
  my $y = $v1->[2]*$v2->[0] - $v1->[0]*$v2->[2];
  my $z = $v1->[0]*$v2->[1] - $v1->[1]*$v2->[0];
  return [$x,$y,$z];
}


#規格化された3ベクトルの作る立体角を求める。
#acosはperlにはない!
sub solid3{
  my ( $ea,$eb,$ec) = @_;
  my @a = normalize(outerprod( $eb, $ec ));
  my @b = normalize(outerprod( $ec, $ea ));
  my @c = normalize(outerprod( $ea, $eb ));
  my $A = $pi - acos(innerprod( \@b, \@c ));
  my $B = $pi - acos(innerprod( \@c, \@a ));
  my $C = $pi - acos(innerprod( \@a, \@b ));
  return $A+$B+$C - $pi;
}


#規格化されていないベクトルの束から立体角を求める。
sub solidn{
  my ( $xyz ) = @_;
  my $n = $#{$xyz} + 1;
  my @e;
  foreach my $i ( 0 .. $n-1 ){
    my @x = normalize( $xyz->[$i] );
    $e[$i] = \@x;
  }
  my $sum = 0;
  foreach my $i ( 1 .. $n-2 ){
    $sum += solid3( $e[0], $e[$i], $e[$i+1] );
  }
  return $sum;
}


sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am solid.\n";
    #if the fragment is not compact.
    if ( $record->{isvitrite} != 0 ){
	return $record->{solid} = 0;
    }

    #print STDERR $record->{shape}, "SHAPE\n";
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
    #determine the "solid" of the ring
    #
    my $solid = 0;
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
	#accumulate the solid angle
	for(my $i=0;$i<$#nodes;$i++){
	  my ($x1,$y1,$z1) = @{$rel[$nodes[$i]]};
	  my ($x2,$y2,$z2) = @{$rel[$nodes[$i+1]]};
	  my ($x3,$y3,$z3) = ($cx,$cy,$cz);
	  #iからの相対
	  $x2 -= $x1;
	  $y2 -= $y1;
	  $z2 -= $z1;
	  $x3 -= $x1;
	  $y3 -= $y1;
	  $z3 -= $z1;
	  #i自身は反転
	  $x1 = -$x1;
	  $y1 = -$y1;
	  $z1 = -$z1;
	  #print STDERR "$x1/$y1/$z1\n";
	  my $sol = solidn( [[$x1,$y1,$z1],
			     [$x2,$y2,$z2],
			     [$x3,$y3,$z3]] );
	  $solid += $sol;  #i
	  #print STDERR $sol,"\n";
	  my ($x1,$y1,$z1) = @{$rel[$nodes[$i+1]]};
	  my ($x2,$y2,$z2) = @{$rel[$nodes[$i]]};
	  my ($x3,$y3,$z3) = ($cx,$cy,$cz);
	  #i+1からの相対
	  $x2 -= $x1;
	  $y2 -= $y1;
	  $z2 -= $z1;
	  $x3 -= $x1;
	  $y3 -= $y1;
	  $z3 -= $z1;
	  #i自身は反転
	  $x1 = -$x1;
	  $y1 = -$y1;
	  $z1 = -$z1;
	  my $sol = solidn( [[$x1,$y1,$z1],
			     [$x2,$y2,$z2],
			     [$x3,$y3,$z3]] );
	  $solid += $sol; #i=1
	  #print STDERR $sol,"\n";
	}
    }
    return $record->{solid} = $solid / ( 4.0 * $pi );
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    if ( $record->{solid} ){
	$html .= $cgi->h2("Solid Angle");
	$html .= $record->{solid};
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
    $xmlg->solid( $record->{solid} );
}



#一覧表示に使えるSQL field名
sub sortkey{
    return "solid";
}



#field名
#"*" indicates it is an approx. value.
sub keylabel{
    return "Solid angle/4 pi steradian*";
}



1;
