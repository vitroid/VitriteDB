package vitrite::crystalviewer;


use strict;
use base qw(vitrite::plugin);
use XML::Generator;
use vitrite::settings;
use vitrite::attrdb qw(ProtectSymbols);
#I don't know why. but it does not keep the variables in settings.
my $BINDIR = "/usr/local/lib/vitrite";


sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = ["graph", "shape"];
    $self->{"inexact"} = 1;   #not useful for looking up a graph
    bless $self, $package;
}

#internal format: scalar
#$crystalviewer = '<?xml version="1.0" ?><crystal><bond site2="7" site1="6" /><bond site2="6" site1="1" /><bond site2="2" site1="1" /><bond site2="7" site1="4" /><bond site2="5" site1="4" /><bond site2="1" site1="0" /><bond site2="5" site1="0" /><bond site2="4" site1="3" /><bond site2="3" site1="2" /><site color="#00ff00" position="-0.366668,-0.572103,-0.822887" id="0" /><site color="#00ff00" position="-0.763321,0.172698,-0.286287" id="1" /><site color="#00ff00" position="-0.821552,-0.102198,0.673421" id="2" /><site color="#00ff00" position="0.094389,-0.309817,1.016855" id="3" /><site color="#00ff00" position="0.763321,-0.172698,0.286287" id="4" /><site color="#00ff00" position="0.549473,-0.779074,-0.479595" id="5" /><site color="#00ff00" position="-0.185758,0.985157,-0.365852" id="6" /><site color="#00ff00" position="0.730116,0.778035,-0.021942" id="7" /></crystal>';


sub name{"crystalviewer";}


sub make {
    my ( $self, $record ) = @_;
    print STDERR "I am crystalviewer.\n";
    my $tmp = "/tmp/crystalviewer$$";

    my $xml = new XML::Generator;
    my $g = $record->{graph};
    my @s;
    foreach my $x ( keys %{$g->{edges}} ){
	foreach my $y ( keys %{$g->{edges}{$x}} ){
	    if ( $g->{directed} || $x < $y ){
		push @s, $xml->bond({site1=>$x, site2=>$y});
	    }
	}
    }
    my $shape = $record->{shape};
    foreach my $i ( 0 .. $shape->{nnode}-1 ){
	my $position = join( ",", @{$shape->{coord}[$i]} );
	push @s, $xml->site({id=>$i, position=>$position, color=>"#00ff00"});
    }
    my $s = $xml->crystal( @s );
    $s = '<?xml version="1.0" ?>' . $s;
    $record->{crystalviewer} = $s;
    return $s;
}



sub to_html{
    my ( $self, $cgi, $record ) = @_;
    my $html;
    $html .= flashviewer( $record->{crystalviewer} );
    $html;
}



sub TagEscape{
    $_ = shift @_;
    s/\&/&amp;/g;
    s/\</&lt;/g;
    s/\>/&gt;/g;
    $_;
}
 
   

sub to_sql{
    my ( $self, $value ) = @_;
    #return $value;
    return $self->name() . "='" . ProtectSymbols($value) . "'";
}



sub from_sql{
    my ( $self, $value ) = @_;
    $value;
}



sub flashviewer{
    my ( $xml ) = @_;
    my $rawxml = TagEscape( $xml );
    my $s = <<EOF
  <noscript>
    <span class="page-noscript">You must have JavaScript enabled to browse
      this library.</span>
  </noscript>
        <h2>Crystal Viewer</h2>
        <object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
		codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0" width="234" height="234" align=""
		ID="crystal">
          <param name=movie value="cv100.swf" />
          <param name=quality value=high />
          <param name=bgcolor value=#FFFFFF />
	  <param name="allowScriptAccess" value="always" />
	  <param name="swfLiveConnect" value="true" />
	  <embed src="/vitrite/cv100.swf"  width="234" height="234" align="" quality=high bgcolor=#FFFFFF NAME="crystal"
		 type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" allowscriptaccess="always" swfliveconnect="true"></embed>
	</object><br />
  <div class="public-relation"><a href="http://www.dawgsdk.org/crystal/"><img src="http://www.dawgsdk.org/crystal/images/banner.gif" alt="Flash Crystal Viewer Banner" width="234" height="30" border="0"></a></div>
  <script language="JavaScript"><!--
				    var InternetExplorer = navigator.appName.indexOf("Microsoft") != -1;
var crystalXML = '$xml';
				    function crystal_DoFSCommand(command, args) {
				    
				    if ((command == "load") && (crystalXML.length > 0)){
  				    ((InternetExplorer) ? document.all.crystal : document.crystal).SetVariable(args, crystalXML);
				    }
				    }
				    //-->
  </script>
  <script language="VBScript">
On Error Resume Next 
Sub crystal_FSCommand(ByVal command, ByVal args)
  Call crystal_DoFSCommand(command, args)
End Sub
  </script>
EOF
;
    return $s;
}



sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    return $record->{crystalviewer};
}



1;


