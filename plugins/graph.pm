package vitrite::graph;


use strict;
use base qw(vitrite::plugin);
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);
#I don't know why. but it does not keep the variables in settings.
my $BINDIR = "/usr/local/lib/vitrite";


sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = [];
    $self->{"inexact"} = 1;   #not useful for looking up a graph
    bless $self, $package;
}

#internal format: hash
#$graph = {
#           'directed' => 1,
#           'edges' => {
#                        '6' => {
#                                 '7' => 1
#                               },
#                        '1' => {
#                                 '6' => 1,
#                                 '2' => 1
#                               },
#                        '4' => {
#                                 '7' => 1,
#                                 '5' => 1
#                               },
#                        '0' => {
#                                 '1' => 1,
#                                 '5' => 1
#                               },
#                        '3' => {
#                                 '4' => 1
#                               },
#                        '2' => {
#                                 '3' => 1
#                               }
#                      },
#           'size' => '8'
#         };



sub name{
    "graph";
}



sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am graph.\n";
    #berkeley DBから抽出するコードをここに入れておけばいいと思う。
    return;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    $html .= $cgi->h2("Graph in \@NGPH format");
    $html .= $cgi->pre( to_ngph( $record->{graph} ) );
    $html;
}


sub to_ngph{
    my ( $g ) = @_;
    my $ngph;
    if ( $g->{size} ){
	$ngph = "\@NGPH\n";
	$ngph .= $g->{size} . "\n";
	foreach my $x ( keys %{$g->{edges}} ){
	    foreach my $y ( keys %{$g->{edges}{$x}} ){
		if ( $g->{directed} || $x < $y ){
		    $ngph .= "$x $y\n";
		}
	    }
	}
	$ngph .= "-1 -1\n";
    }
    $ngph;
}



sub from_ngph{
    my ( @s ) = @_;
    #もし行で分割したarrayではないものが渡されたら、分割してみる。(fault torrerance)
    if ( $#s == 0 ){
	@s = split( /\n/, $s[0] );
    }
    my $g;
    while( 0<=$#s ){
	my $line = shift @s;
	if ( $line =~ /^\@NGPH/ ){
	    my $g;
	    $g->{size} = shift @s;
	    $g->{directed}=1;
	    while( 0<=$#s ){
		$_ = shift @s;
		chomp;
		my ( $x, $y ) = split;
		last if $x < 0;
		$g->{edges}{$x}{$y}=1;
	    }
	    return $g;
	}
    }
    $g;
}




#
#Allow simplefied notations
#e.g. A-B, C-D, A-D, C-B
#     A:B C:D E:F
#Expression must be in one line.
#where "-" "=", or ":" is recognized as an edge.
#combination of "," and " " is recognized as a separator.
#any label is allowed.
sub from_simple{
    my ( $s ) = @_;
    #map label to nude number
    my %directory;
    my $nnode = 0;
    #split into pairs
    my @s = split( /[, ]+/, $s );
    my $g;
    foreach my $pair ( @s ){
	my @nodes = split( /[-=:.]/, $pair );
	#single node is not allowed, while a chain of nodes is allowed.
	if ( $#nodes < 1 ){
	    # it will cause error, anyway.
	    $g->{size}=0;
	    undef $g->{edges};
	    return $g;
	}
	my $x;
	my $label = shift @nodes;
	if ( defined $directory{$label} ){
	    $x = $directory{$label};
	}
	else{
	    $directory{$label} = $nnode;
	    $x = $nnode;
	    $nnode++;
	}
	while ( $#nodes >= 0 ){
	    my $y;
	    $label = shift @nodes;
	    if ( defined $directory{$label} ){
		$y = $directory{$label};
	    }
	    else{
		$directory{$label} = $nnode;
		$y = $nnode;
		$nnode++;
	    }
	    print STDERR "$x-$y\n";
	    $g->{edges}{$x}{$y}=1;
	    $x = $y;
	}
    }
    $g->{size} = $nnode;
    $g->{directed} = 1;
    $g;
}

    

sub to_sql{
    my ( $self, $value ) = @_;
    $value = to_ngph( $value );
    return $self->name() . "='" . ProtectSymbols($value) . "'";
}



sub from_sql{
    my ( $self, $value ) = @_;
    return "" if $value eq "";
    from_ngph( split(/\n/, $value) );
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    my $g = $record->{graph};
    my @s;
    foreach my $x ( keys %{$g->{edges}} ){
	foreach my $y ( keys %{$g->{edges}{$x}} ){
	    if ( $g->{directed} || $x < $y ){
		push @s, $xmlg->bond({site1=>$x, site2=>$y});
	    }
	}
    }
    $xmlg->graph( {size=>$g->{nnode}}, @s );
}



1;
