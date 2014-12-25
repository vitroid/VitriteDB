# -*- condig: utf-8; -*-
package vitrite::shape;


use strict;
use base qw(vitrite::plugin);
use vitrite::graph;
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);
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
#$shape = {
#           'coord' => [
#                        [
#                          '-0.366668',
#                          '-0.572103',
#                          '-0.822887'
#                        ],
#                        [
#                          '-0.763321',
#                          '0.172698',
#                          '-0.286287'
#                        ],
#                        [
#                          '-0.821552',
#                          '-0.102198',
#                          '0.673421'
#                        ],
#                        [
#                          '0.094389',
#                          '-0.309817',
#                          '1.016855'
#                        ],
#                        [
#                          '0.763321',
#                          '-0.172698',
#                          '0.286287'
#                        ],
#                        [
#                          '0.549473',
#                          '-0.779074',
#                          '-0.479595'
#                        ],
#                        [
#                          '-0.185758',
#                          '0.985157',
#                          '-0.365852'
#                        ],
#                        [
#                          '0.730116',
#                          '0.778035',
#                          '-0.021942'
#                        ]
#                      ],
#           'nnode' => 8,
#           'rd' => '0.000000'
#         };



sub name{"shape";}

sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am shape.\n";

    my $tmp = "/tmp/shape$$";
    open OUT, ">$tmp";
    print OUT vitrite::graph::to_ngph( $record->{graph} );
    close OUT;
    open IN, "$BINDIR/totalgraphquench.sh $tmp |";
    my @lines = <IN>;
    close IN;
    unlink $tmp;
    #print STDERR join(" ",@lines), "\n";
    chomp @lines;
    my $shape = from_ard0ar3a( @lines );
    $record->{shape} = $shape;
    return $shape;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    #$html .= $cgi->h2("Residual Distortion");
    #$html .= $record->{shape}{rd};
    $html .= $cgi->h2("Coordinates in \@AR3A format");
    $html .= $cgi->pre( to_ar3a( $record->{shape} ) );
    $html;
}


sub to_ar3a{
    my ( $shape ) = @_;
    my $s;
    $s    .= "\@AR3A\n";
    $s    .= $shape->{nnode} . "\n";
    foreach my $i ( 0 .. $shape->{nnode}-1 ){
	$s .= join( " ", @{$shape->{coord}[$i]} ) . "\n";
    }
    $s;
}



sub to_ard0{
    my ( $shape ) = @_;
    my $s;
    $s    .= "\@ARD0\n";
    $s    .= $shape->{rd} . "\n";
    $s;
}



sub from_ard0ar3a{
    my ( @s ) = @_;
    my $shape;
    while( 0<=$#s ){
	$_ = shift @s;
	if ( /^\@ARD0/ ){
	    $shape->{rd} = shift @s;
	}
	elsif ( /^\@AR3A/ ){
	    $shape->{nnode} = shift @s;
	    foreach my $i ( 0.. $shape->{nnode}-1 ){
		$_ = shift @s;
		chomp;
		$shape->{coord}[$i] = [ split /\s+/, $_ ];
	    }
	}
    }
    #print STDERR to_ard0( $shape ) . " ARD0\n";
    #print STDERR to_ar3a( $shape ) . " AR3A\n";
    $shape;
}

    

sub to_sql{
    my ( $self, $value ) = @_;
    $value = to_ard0( $value ) . to_ar3a( $value );
    return $self->name() . "='" . ProtectSymbols($value) . "'";
}



sub from_sql{
    my ( $self, $value ) = @_;
    return "" if $value eq "";
    from_ard0ar3a( split(/\n/, $value) );
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    my $shape = $record->{shape};
    my @s;
    foreach my $i ( 0 .. $shape->{nnode}-1 ){
	my $position = join( ",", @{$shape->{coord}[$i]} );
	push @s, $xmlg->site({id=>$i}, $position);
    }
    return $xmlg->shape({size=>$shape->{nnode}}, @s);
}




#sub sortkey{
#    return "rd";
#}



#"*" indicates it is an approx. value.
#sub keylabel{
#    return "Residual Distortion*";
#}



1;
