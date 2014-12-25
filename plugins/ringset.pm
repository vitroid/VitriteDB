package vitrite::ringset;


use strict;
use base qw(vitrite::plugin);
use vitrite::graph;
use vitrite::attrdb qw(ProtectSymbols);
use XML::Generator;
#use vitrite::settings;
#I don't know why. but it does not keep the variables in settings.
my $BINDIR = "/usr/local/lib/vitrite";


sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["graph"];
    $self->{"inexact"} = 1;   #not useful for looking up a graph
    bless $self, $package;
}

#internal format: hash
#$ringset = {
#             'ring' => [
#                         [
#                           '0',
#                           '1',
#                           '2',
#                           '3',
#                           '4',
#                           '5'
#                         ],
#                         [
#                           '0',
#                           '1',
#                           '6',
#                           '7',
#                           '4',
#                           '5'
#                         ],
#                         [
#                           '1',
#                           '2',
#                           '3',
#                           '4',
#                           '7',
#                           '6'
#                         ]
#                       ],
#             'size' => 8
#           };



sub name{
    "ringset";
}



sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am ringset.\n";

    my $tmp = "/tmp/ringset$$";
    open OUT, ">$tmp";
    print OUT vitrite::graph::to_ngph( $record->{graph} );
    close OUT;
    #print encode_ngph( $g );
    open IN, "$BINDIR/countrings2 -C 8 < $tmp |";
    my @lines = <IN>;
    close IN;
    unlink $tmp;

    my $ringset = from_rngs( @lines );
    $record->{ringset} = $ringset;
    return $ringset;
}



sub from_rngs{
    my ( @s ) = @_;
    if ( $#s == 0 ){
	@s = split( /\n/, $s[0] );
    }

    my $rngs;
    while( 0 <= $#s ){
	$_ = shift @s;
	if ( /^\@RNGS/ ){
	    $rngs->{size} = 0 + shift @s;
	    while( 0<= $#s ){
		$_ = shift @s;
		split;
		my $size = shift @_;
		last if $size <= 0;
		push @{$rngs->{ring}}, [ @_ ];
	    }
	    return $rngs;
	}
    }
    return;
}



sub to_rngs{
    my ( $rngs ) = @_;
    return unless defined $rngs;
    my $s;
    $s .= "\@RNGS\n";
    $s .= $rngs->{size} . "\n";
    foreach my $ring ( @{$rngs->{ring}} ){
	$s .= join(" ", $#{$ring}+1, @{$ring}) . "\n";
    }
    $s .= "0\n";
    $s;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    $html .= $cgi->h2("Ring set in \@RNGS format");
    $html .= $cgi->pre( to_rngs( $record->{ringset}));
    $html;
}


#from internal data format to plain text for sql
sub to_sql{
    my ( $self, $value ) = @_;
    $value = to_rngs( $value );
    return $self->name() . "='" . ProtectSymbols($value) . "'";
}

    

#from plain text for sql to internal data format
sub from_sql{
    my ( $self, $value ) = @_;
    from_rngs( $value );
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    my $r = $record->{ringset};
    my @t;
    foreach my $ring ( @{$r->{ring}} ){
	my @s;
	foreach my $i ( 0..$#{$ring} ){
	    push @s, $xmlg->node($ring->[$i]);
	}
	push @t, $xmlg->ring({size=>$#{$ring}+1}, @s);
    }
    return $xmlg->ringset({size=>$#{$r->{ring}}+1}, @t);
}
    


1;
