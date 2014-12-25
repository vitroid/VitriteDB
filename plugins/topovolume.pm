package vitrite::topovolume;

use strict;
use base qw(vitrite::plugin);
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);

sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["graph", "isvitrite"];
    bless $self, $package;
}

#internal format: scalar
#$topovolume = '0.5';


sub name{"topovolume";}

sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am topovolume.\n";
    #if the fragment is not compact.
    if ( $record->{isvitrite} != 0 ){
	return $record->{topovolume} = 0;
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
	elsif ( $conn[$i] != 2 ){
	    return $record->{topovolume} = 0;
	}
    }
    return $record->{topovolume} = $n3/4.0;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    if ( $record->{topovolume} ){
	$html .= $cgi->h2("Topological Volume");
	$html .= $record->{topovolume};
    }
    $html;
}



sub to_sql{
    my ( $self, $value ) = @_;
    #$value;
    return $self->name() . "=" . ProtectSymbols($value);
}



sub from_sql{
    my ( $self, $value ) = @_;
    return "" if $value eq "";
    $value;
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    $xmlg->topovolume( $record->{topovolume} );
}



#一覧表示に使えるSQL field名
sub sortkey{
    return "topovolume";
}



#field名
sub keylabel{
    return "Topological Volume";
}



1;
