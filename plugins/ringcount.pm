package vitrite::ringcount;


use strict;
use base qw(vitrite::plugin);
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);


sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["ringset"];
    bless $self, $package;
}

#internal format: array of int; [0..5] corresponds to 3- to 8-rings.
#$ringcount = [
#               '0',
#               '0',
#               '0',
#               '3',
#               '0',
#               '0'
#             ];


sub name{
    "ringcount";
}



sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am ringcount.\n";
    my @ringcount;
    foreach my $ring ( @{$record->{ringset}{ring}} ){
        my $size = 1 + $#{$ring};
        $ringcount[$size]++;
    }
    shift @ringcount;
    shift @ringcount;
    shift @ringcount;
    for(my $i=0; $i<=8-3;$i++){
        $ringcount[$i] ||= 0;
    }
    
    $record->{ringcount} = [ @ringcount ];
    return $record->{ringcount};
    #print STDERR $record->{ringcount}, " RINGCOUNT\n";
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    $html .= $cgi->h2("Ring count");
    my @ringcount = @{$record->{ringcount}};
    my @el;
    foreach my $i ( 0 .. $#ringcount ){
        push @el, $cgi->dt($i+3)
            .     $cgi->dd($ringcount[$i]);
    }
    $html .= $cgi->dl(@el);
    $html;
}



sub to_sql{
    my ( $self, $value ) = @_;
    my @s;
    foreach my $i ( 3..8 ){
        push @s, "ring$i='" . ProtectSymbols($value->[$i-3]) . "'";
    }
    push @s, $self->name() . "='" . ProtectSymbols(join(" ", @{$value})) . "'";
    return join(",", @s);
}



sub from_sql{
    my ( $self, $value ) = @_;
    return "" if $value eq "";
    [ split( / /, $value) ];
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    my $r = $record->{ringcount};
    my @s;
    foreach my $i ( 0..5 ){
        push @s, $xmlg->count({size=>$i+3},$r->[$i]);
    }
    return $xmlg->ringcount(@s);
}



#一覧表示に使えるSQL field名
sub sortkey{
    return ( "ring3", "ring4", "ring5", "ring6", "ring7", "ring8" );
}



#field名
sub keylabel{
    return ( "3-ring", "4-ring", "5-ring", "6-ring", "7-ring", "8-ring" );
}



1;
