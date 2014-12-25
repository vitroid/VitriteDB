package vitrite::spectrum;


use strict;
use FileHandle;
use IPC::Open2;
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
    #$self->{"inexact"} = 1;
    bless $self, $package;
}

#internal format: scalar int



sub name{
    "spectrum";
}



sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am spectrum.\n";
    #bidir pipe
    my $pid = open2(*Reader, *Writer, "$BINDIR/spectrum" );
    Writer->autoflush(); # default here, actually
    print Writer vitrite::graph::to_ngph( $record->{graph} );
    close Writer;
    my $spectrum = <Reader>;
    close Reader;
    $record->{spectrum} = $spectrum;
    return $spectrum;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    if ( $record->{spectrum} ){
	$html .= $cgi->h2("Hash");
	$html .= $record->{spectrum};
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
    $xmlg->spectrum( $record->{spectrum} );
}


#一覧表示に使えるSQL field名
#sub sortkey{
#    return "spectrum";
#}



#field名
#sub keylabel{
#    return "Hash";
#}


1;
