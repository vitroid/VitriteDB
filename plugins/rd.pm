# -*- condig: utf-8; -*-
package vitrite::rd;

use strict;
use base qw(vitrite::plugin);
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);

sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["shape"];
    $self->{"inexact"} = 1;   #not useful for looking up a graph
    bless $self, $package;
}

#internal format: scalar
#$rd = '0.684266818838192';

sub name{"rd";}


sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am rd.\n";
    my $rd = $record->{shape}{rd};
    return $record->{rd} = $rd;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    if ( $record->{rd} ){
	$html .= $cgi->h2("Residual Distortion");
	$html .= $record->{rd};
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
    $xmlg->rd( $record->{rd} );
}



#一覧表示に使えるSQL field名
sub sortkey{
    return "rd";
}



#field名
#"*" indicates it is an approx. value.
sub keylabel{
    return "Residual Distortion*";
}



1;
