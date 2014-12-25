package vitrite::eulerindex;


use strict;
use base qw(vitrite::plugin);
use vitrite::graph;
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);


sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["graph", "ringcount"];
    bless $self, $package;
}

#internal format: array of int.
#[0] : r -e + n
#[1] : number of rings
#[2] : number of edges
#[3] : number of nodes
#$eulerindex = [
#                '2',
#                '3',
#                '9',
#                '8'
#              ];


sub name{
    "eulerindex";
}


sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am eulerindex.\n";
    my @eulerindex;
    my ($nring, $nnode, $nedge);
    my $g = $record->{graph};
    $nnode = $g->{size};
    foreach my $x ( keys %{$g->{edges}} ){
	foreach my $y ( keys %{$g->{edges}{$x}} ){
	    if ( $g->{directed} || $x < $y ){
		$nedge ++;
	    }
	}
    }
    my @r = @{$record->{ringcount}};
    $nring = $r[0] + $r[1] + $r[2] + $r[3] + $r[4] + $r[5];
    my $eulerindex = $nring - $nedge + $nnode;
    
    $record->{eulerindex} = [ $eulerindex, $nring, $nedge, $nnode ];
    return $record->{eulerindex};
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    $html .= $cgi->h2("Euler's formula");
    my @eulerindex = @{$record->{eulerindex}};
    my @el;
    push @el, $cgi->dt("Euler's index") . $cgi->dd($eulerindex[0]);
    push @el, $cgi->dt("number of rings") . $cgi->dd($eulerindex[1]);
    push @el, $cgi->dt("number of edges") . $cgi->dd($eulerindex[2]);
    push @el, $cgi->dt("number of vertices") . $cgi->dd($eulerindex[3]);
    $html .= $cgi->dl(@el);
    $html;
}



#書き込みの際に、配列要素を別のフィールドに保存する。
#検索対策。
sub to_sql{
    my ( $self, $value ) = @_;
    my @s;
    foreach my $i ( 0..3 ){
	push @s, "topo$i='" . ProtectSymbols($value->[$i]) . "'";
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
    #no output in XML
    return $xmlg->eulerindex($record->{topo0});
}



#一覧表示に使えるSQL field名
sub sortkey{
    return ( "topo0", "topo1", "topo2", "topo3" );
}



#field名
sub keylabel{
    return ( "Euler Index","Number of Rings","Number of Edges","Number of Vertices" );
}



1;
