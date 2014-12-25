package vitrite::image;


use strict;
use base qw(vitrite::plugin);
use vitrite::graph;
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);
#I don't know why. but it does not keep the variables in settings.
my $MYNAME="vitrite.cgi";
my $RELURL = "/cgi-bin/$MYNAME";
my $BINDIR = "/usr/local/lib/vitrite";
my $IMAGEPATH = "/var/www/vitrite";
my $IMAGEURL = "/vitrite";


sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["graph", "shape"];
    $self->{"inexact"} = 1;   #not useful for looking up a graph
    bless $self, $package;
}

#internal format: scalar
#$image = '8';
#the image path relative to $IMAGEPATH

sub name{"image";}



sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am image.\n";

    my $id = $record->{id};
    my @path = reverse split( //, $id );
    pop @path;
    my $path = join("/", @path, $id );
    my $pov  = $IMAGEPATH . "/" . $path . ".pov";
    my $png  = $IMAGEPATH . "/" . $path . ".png";
    recursive_mkdir( $IMAGEPATH, @path );

    open OUT, ">$pov";
    print OUT <<EOF;
light_source {<0,0,100> color rgb<1,1,1>}
camera {
  location <0,0,10>
  angle 30
  up    <0,1,0>
  right <1,0,0>
  look_at  <0, 0,  0>
}
background { color rgb<1,1,1> }
\#declare Soft_Silver =
texture {
  pigment { color rgb <0.5, 1, 1> }
  finish {
    metallic
    ambient 0.2
    diffuse 0.35
    specular 0.85
    //roughness 0.01
    //reflection 0.45
    brilliance 1.5
  }
}
\#declare TNODE=Soft_Silver;
\#declare TBOND=TNODE;
\#declare RNODE=0.1;
\#declare RBOND=0.05;

EOF
    ;
    my $shape = $record->{shape};
    foreach my $coord ( @{$shape->{coord}} ){
	print OUT "sphere{<",
	join( ",", @{$coord} ),
	">, RNODE texture {TNODE}}\n";
    }
    my $g = $record->{graph};
    foreach my $x ( keys %{$g->{edges}} ){
	foreach my $y ( keys %{$g->{edges}{$x}} ){
	    if ( $g->{directed} || $x < $y ){
		my $coordx = $shape->{coord}[$x];
		my $coordy = $shape->{coord}[$y];
		print OUT "cylinder{<",
		join( ",", @{$coordx} ),
		">,<",
		join( ",", @{$coordy} ),
		"> RBOND open no_shadow hollow texture {TBOND}}\n";
	    }
	}
    }

    system( "povray +w160 +h160 +FN +Q9 +A0.3 +AM2 -D0 -i$pov -O$png 1>/dev/null 2>/dev/null" );
    return $record->{image} = $path;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    my $id = $record->{id};
    $html .= $cgi->h2("Image");
    $html .= $cgi->a({-href=>"$RELURL?id=$id"},
		     $cgi->img({-src=>$IMAGEURL . "/" . $record->{image} . ".png"}));
    $html;
}

    

sub to_sql{
    my ( $self, $value ) = @_;
    #return $value;
    return $self->name() . "='" . ProtectSymbols($value) . "'";
}



sub from_sql{
    my ( $self, $value ) = @_;
    return "" if $value eq "";
    $value;
}



sub recursive_mkdir{
    my ( $base, @path ) = @_;
    my @stack;
    while( 0 <= $#path ){
	unshift @stack, join( "/", $base, @path );
	pop @path;
    }
    foreach my $dir ( @stack ){
	print STDERR "MKDIR $dir\n";
	mkdir($dir, 0755);
    }
}

	

sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    return $xmlg->image( { src=>$IMAGEURL . "/" . $record->{image} . ".png" } );
}



1;
