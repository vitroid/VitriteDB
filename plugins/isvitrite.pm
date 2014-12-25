package vitrite::isvitrite;

use strict;
use base qw(vitrite::plugin);
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);

sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["graph", "eulerindex", "ringset"];
    bless $self, $package;
}

#internal format: scalar
#$isvitrite = '0';
# 0 indicates it is vitrite
# 1 indicates it is not because euleridex is not 2
# 2 indicates it is not because some node has more than 3 edges
# 4 indicates it is not because some node has less than 2 edges (dangling bonds)
# 8 indicates it is not because some bond is not shared by rings.
# 16 indicates it is not because some bond is shared by more than two rings.

sub name{"isvitrite";}

sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am isvitrite.\n";
    my $status = 0;
    #if the fragment is not compact.
    if ( $record->{eulerindex}[0] != 2 ){
	$status |= 1;
    }
    #count the connection index
    my @conn;
    my $g = $record->{graph};
    my $nnode = $g->{size};
    foreach my $x ( keys %{$g->{edges}} ){
	foreach my $y ( keys %{$g->{edges}{$x}} ){
	    if ( $g->{directed} || $x < $y ){
		$conn[$x] ++;
		$conn[$y] ++;
	    }
	}
    }
    my $n3;
    foreach my $i ( 0.. $#conn ){
	if ( $conn[$i] == 3 ){
	    $n3 ++;
	}
	elsif ( $conn[$i] < 2 ){
	    $status |= 4;
	}
	elsif ( 3 < $conn[$i]  ){
	    $status |= 2;
	}
    }

    #すべての辺が2つの環に共有されていることを保証する。
    my %bonds;
    foreach my $ring ( @{$record->{ringset}{ring}} ){
	my @nodes = @{$ring};
	push @nodes, $nodes[0];
	foreach my $i ( 1..$#nodes ){
	    #doubly marked
	    $bonds{$nodes[$i-1]}{$nodes[$i]} ++;
	    $bonds{$nodes[$i]}{$nodes[$i-1]} ++;
	}
    }
    foreach my $x ( values %bonds ){
	foreach my $y ( values %{$x} ){
	    if ( $y > 2 ){
		$status |= 16;
	    }
	    elsif ( $y < 2 ){
		$status |= 8;
	    }
	}
    }
    print STDERR "New status: $status\n";
    return $record->{isvitrite} = $status;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    my $status = $record->{isvitrite};
    if ( $status == 0 ){
	$html .= "This graph is a vitrite.";
    }
    else{
	$html .= "This graph is NOT a vitrite because";
	my @s;
	if ( $status & 1 ){
	    push @s, "Euler's index is not 2."
	}
	if ( $status & 2 ){
	    push @s, "The graph has node(s) with more than 3 edges."
	}
	if ( $status & 4 ){
	    push @s, "The graph has node(s) with less than 2 edges."
	}
	if ( $status & 8 ){
	    push @s, "The graph has bond(s) not shared by rings."
	}
	if ( $status & 16 ){
	    push @s, "The graph has node(s) shared by more than 2 rings."
	}
	$html .= $cgi->ul( $cgi->li( [ @s ] ));
    }
    $cgi->p($html);
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
    $xmlg->isvitrite( $record->{isvitrite} );
}



#一覧表示に使えるSQL field名
sub sortkey{
    return "isvitrite";
}



#field名
sub keylabel{
    return "Is a vitrite?(0=yes)";
}



1;
