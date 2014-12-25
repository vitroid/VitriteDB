package vitrite::symmetry;


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
    bless $self, $package;
}

#internal format: scalar int



sub name{
    "symmetry";
}



sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am symmetry.\n";

    my $tmp = "/tmp/symmetry$$";
    open OUT, ">$tmp";
    print OUT vitrite::graph::to_ngph( $record->{graph} );
    close OUT;
    open IN, "$BINDIR/symmetry -c $tmp |";
    my $symmetry = <IN>;
    $symmetry += 0;
    close IN;
    unlink $tmp;

    $record->{symmetry} = $symmetry;
    return $symmetry;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    if ( $record->{symmetry} ){
	$html .= $cgi->h2("Symmetry Number");
	$html .= $record->{symmetry};
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
    $xmlg->symmetry( $record->{symmetry} );
}


#一覧表示に使えるSQL field名
sub sortkey{
    return "symmetry";
}



#field名
sub keylabel{
    return "Symmetry Number";
}


1;
