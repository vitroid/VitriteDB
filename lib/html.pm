package vitrite::html;

use strict;
use CGI;
use XML::Generator;
BEGIN{
    use Exporter ();
    use vars qw(@ISA @EXPORT @EXPORT_OK);
    
    @ISA       = qw(Exporter);
    @EXPORT    = qw(Header Footer to_html to_xml Query Query2 Column );
    @EXPORT_OK = qw();
}

use vars @EXPORT_OK;

#constants
my $MYNAME="vitrite.cgi";
my $RELURL = "/cgi-bin/$MYNAME";
#my $HOSTNAME = "vitrite.chem.nagoya-u.ac.jp";
#my $ABSURL = "http://$HOSTNAME$RELURL";
my $CSSURL = "/vitrite/CSS/vitrite.css";
my $DBDIR  = "/var/db/vitrite";
my $BINDIR = "/usr/local/lib/vitrite";
my $IMAGEPATH = "/var/www/vitrite";
my $IMAGEURL = "/vitrite";



#copied from TimeAccount
sub Footer{
    my ( $cgi ) = @_;
    my $html;
    $html .= $cgi->end_html();
    $html;
}

sub Header
{
    my ( $cgi, $title, $session )=@_;
    my $html;
    #
    #record session id as a cookie.
    #
    #$html .= $session->header(-charset=>"utf-8" );
    $html .= $cgi->header(-charset=>"utf-8" );

    my @head;
    push @head, $cgi->Link({-rel=>"stylesheet",
			  -href=>$CSSURL,
			  -type=>"text/css"});


    $html .= $cgi->start_html(-title=>"$title",
			    -lang =>"ja",
			    -encoding=>"UTF-8",
			    -head => [ @head ]
			    );
    $html;
}


#Query window
sub Query{
    my ( $cgi ) = @_;

    my $html;
    $html .= 
	$cgi->start_form(-method=>"POST",
			 -action=>$RELURL,
			 -enctype=>"multipart/form-data"
	)
	.    "Graph ID"
	.    $cgi->textfield( -name=>"id", -size=>30 )
	.    $cgi->submit("q","search by id" )
	.    $cgi->br
	.    $cgi->end_form()

	.    $cgi->start_form(-method=>"POST",
			      -action=>$RELURL,
			      -enctype=>"multipart/form-data"
	)
	.    $cgi->span({-class=>"term"},
		     $cgi->dfn("GRAPH in simple format" . 
		     $cgi->span({-class=>"annotation"}, 'The followings are example expressions of a pentagon:<br /><ol><li>1:2:3 3:4:5 5:1</li><li>A-B,B-C,C-D,D-E,E-A</li><li>Dream.Rouge.Lemonade.Mint.Aqua.Dream</li></ol>You can use "-", ":", or "." for expressing an edge between the vertices, and "," or " " is recognized as a separator.')))
	.    $cgi->textfield( -name=>"simple", -size=>100 )
	.    $cgi->submit( -name=>"search" )
	.    $cgi->br
	.    $cgi->end_form()

	.    $cgi->start_form(-method=>"POST",
			      -action=>$RELURL,
			      -enctype=>"multipart/form-data"
	)
	.    $cgi->span({-class=>"term"},
		     $cgi->dfn("GRAPH in NGPH format" . 
		     $cgi->span({-class=>"annotation"}, "This query is under construction.")))
	.    $cgi->textarea( -name=>"ngph" )
	.    $cgi->submit("q","search" )
	.    $cgi->br
	.    $cgi->end_form();
    $html;
}



sub to_html{
    my ( $cgi, $record, $plugins ) = @_;
    my $html;
    return unless defined $record;
    $html .= $cgi->h1("ID");
    $html .= $record->{id};
    if ( $record->{id} == 0 ){
	$html .= " (not found)";
    }
    if ( defined $plugins->{isvitrite} && defined $record->{isvitrite} ){ $html .= $plugins->{isvitrite}->to_html( $cgi, $record ) };
    if ( defined $plugins->{spectrum} && defined $record->{spectrum} ){ $html .= $plugins->{spectrum}->to_html( $cgi, $record ) };
    #
    #idが0の場合は、dbに保存されないグラフなので、shapeまでは表示しない。
    #

    $html .= $cgi->h1("Shape");
    my @s;
    foreach my $field ( "image", "crystalviewer" ){
	if ( defined $plugins->{$field} && defined $record->{$field}){ push @s, $plugins->{$field}->to_html( $cgi, $record ) };
    }
    $html .= $cgi->table( $cgi->Tr( $cgi->td( [ @s ] ) ) );
    foreach my $field ( "rd", "shape", "volume", "solid" ){
	if ( defined $plugins->{$field} && defined $record->{$field}){ $html .= $plugins->{$field}->to_html( $cgi, $record ) };
    }

    $html .= $cgi->h1("Topology");
    foreach my $field ( "graph", "ringset", "ringcount" ){
	if ( defined $plugins->{$field} && defined $record->{$field}){ $html .= $plugins->{$field}->to_html( $cgi, $record ) };
    }

    $html .= $cgi->h1("Compactness");
    foreach my $field ( "eulerindex", "topovolume", "symmetry" ){
	if ( defined $plugins->{$field} && defined $record->{$field}){ $html .= $plugins->{$field}->to_html( $cgi, $record ) };
    }
    $html;
}



sub to_xml{
    my ( $record, $plugins, @fields ) = @_;
    my @s;
    my $xmlg = new XML::Generator;
    foreach my $field ( @fields ){
	if ( $field ne "id" ){
	    push @s, $plugins->{$field}->to_xml( $xmlg, $record );
	}
    }
    return $xmlg->vitrite( {id=>$record->{id}}, @s );
}






#sub Dump{
#    use Data::Dumper;
#    my ( $id ) = @_;
#    my $q = new CGI;
#    my $db = attrdb_open( "vitrite", "matto", "vitrite" );
#    my @fields = set_fields( $q, $plugins );
#    my $record = attrdb_scan2( $db, $id, $plugins, @fields );
#    attrdb_close($db);
#    foreach my $field ( @fields ){
#	#print STDERR $field, $record->{$field}, "\n";
#	my $d = Data::Dumper->new( [ $record->{$field} ], [ $field ] );
#	my $result = $d->Dump();
#	print $result;
#    }
#    exit 0;
#}









sub Query2{
    my ( $cgi, $plugins ) = @_;

    #queryに使えるキーを、各pluginに問いあわせる。
    my @keys = ("id");
    foreach my $plugin ( values %{$plugins} ){
	push @keys, $plugin->sortkey();
    }
    #キーの名称を、各pluginに問いあわせる。
    my @labels = ("ID");
    foreach my $plugin ( values %{$plugins} ){
	push @labels, $plugin->keylabel();
    }
    
    my %labels;
    foreach my $i ( 0..$#keys ){
	$labels{$keys[$i]} = $labels[$i];
    }

    my %operators = ( "eq"=>"==",
		      "ne"=>"!=",
		      "lt"=>"<",
		      "gt"=>">",
		      "in"=>"in"
	);
    my @operators = ( "eq", "ne", "lt", "gt", "in" );

    my %attributes = ();

    my $html;
    $html .= $cgi->start_form(-method=>"POST",
			      -action=>$RELURL,
			      -enctype=>"multipart/form-data"
	);
    $html .= $cgi->popup_menu(-name=>'key1',
			      -values=>[ @keys ],
			      -default=>'id',
			      -labels=>\%labels);
    $html .= $cgi->popup_menu(-name=>'op1',
			      -values=>[ @operators ],
			      -default=>'eq',
			      -labels=>\%operators);
    $html .= $cgi->textfield(-name=>'val1', -size=>30);
    $html .= " and" . $cgi->br;
    $html .= $cgi->popup_menu(-name=>'key2',
			      -values=>[ @keys ],
			      -default=>'id',
			      -labels=>\%labels);
    $html .= $cgi->popup_menu(-name=>'op2',
			      -values=>[ @operators ],
			      -default=>'eq',
			      -labels=>\%operators);
    $html .= $cgi->textfield(-name=>'val2', -size=>30);
    $html .= " and" . $cgi->br;
    $html .= $cgi->popup_menu(-name=>'key3',
			      -values=>[ @keys ],
			      -default=>'id',
			      -labels=>\%labels);
    $html .= $cgi->popup_menu(-name=>'op3',
			      -values=>[ @operators ],
			      -default=>'eq',
			      -labels=>\%operators);
    $html .= $cgi->textfield(-name=>'val3', -size=>30);
    $html .= " and" . $cgi->br;
    $html .= $cgi->popup_menu(-name=>'key4',
			      -values=>[ @keys ],
			      -default=>'id',
			      -labels=>\%labels);
    $html .= $cgi->popup_menu(-name=>'op4',
			      -values=>[ @operators ],
			      -default=>'eq',
			      -labels=>\%operators);
    $html .= $cgi->textfield(-name=>'val4', -size=>30);
    $html .= " and" . $cgi->br;
    $html .= $cgi->popup_menu(-name=>'key5',
			      -values=>[ @keys ],
			      -default=>'id',
			      -labels=>\%labels);
    $html .= $cgi->popup_menu(-name=>'op5',
			      -values=>[ @operators ],
			      -default=>'eq',
			      -labels=>\%operators);
    $html .= $cgi->textfield(-name=>'val5', -size=>30);
    $html .= $cgi->br;
    $html .= $cgi->submit(-name=>'q', -value=>"Search");
    $html .= $cgi->end_form();
    $html .= $cgi->p("Specification format for 'in' operator: '5 6 7 9-20' or '5,6,7,9-20' or '5-7,9:20' ");
    return $html;
}



sub Column{
    my ( $cgi, $plugins ) = @_;

    my $column;
    $column .= $cgi->h1("Vitrite ( Network Motif of Water ) Database");
    $column .= $cgi->p("Vitrite is the typical network motif of water at low temperature. It is defined as a graph satisfying the following conditions:");
    $column .= $cgi->ol( $cgi->li( [ "Each vertex must be 2- or 3-connected.",
				     " Each edge must be shared by two rings.",
				     "Consists of 3- to 8-membered rings.",
				     "Must satisfy the following Euler's formula for planar graph:" 
				     . $cgi->br 
				     . "F - E + V = 2" 
				     . $cgi->br 
				     . "where F, E, V are number of rings, edges, and vertices, respectively." ] ) );
    $column .= $cgi->p("Note that not all the graphs in this database satisfy these conditions. Volume and topological volume are not shown for the graph not satisfying the conditions.");
    $column .= $cgi->h2("How to get a graph in XML.");
    $column .= $cgi->p("Here is a sample URL to get a graph and its topological/geometrical attributes in XML:");
    my $url = "$RELURL?id=39&form=xml&field=ringset,shape";
    $column .= $cgi->a({-href=>$url}, "Sample Query");
    $column .= $cgi->p("The parameter 'id' specifies the graph ID in the database. You can specify multiple attributes for 'field' parameter in comma-separated format. Currently you can specify the following fields:");
    my @keys;
    foreach my $plugin ( values %{$plugins} ){
	push @keys, $plugin->name();
    }
    $column .= $cgi->ol( $cgi->li( [ @keys ] ) );
    $column .= $cgi->p("Note that 'graph' is mandatory.");
    $column .= $cgi->h2("How to get the results by narrowing search in XML.");
    $column .= $cgi->p("To get a set of graphs satisfying a couple of conditions at a time, a sample URL is following:");
    $url = "$RELURL?form=xml&field=graph,isvitrite&key1=id&op1=in&val1=3,5,7-9&key2=isvitrite&op2=eq&val2=0";
    $column .= $cgi->a({-href=>$url}, "Sample Query");
    $column .= $cgi->p("Number of query results can be limited by keywords 'sta'(starts) and 'rec'(records). If you need only the first 50 records out of 1000 query results, add 'sta=0&rec=50' to the URL. if you want the second 50 records, it becomes 'sta=50&rec=50'. If rec is unspecified, number of query results is limited to 1000 for fail-safe. See the following sample queries.");
    $url = "$RELURL?form=xml&field=isvitrite&key1=isvitrite&op1=eq&val1=0&sta=0&rec=10";
    $column .= $cgi->p($cgi->a({-href=>$url}, "Sample Query 1 (first 10 matches)"));
    $url = "$RELURL?form=xml&field=isvitrite&key1=isvitrite&op1=eq&val1=0&sta=10&rec=9";
    $column .= $cgi->p($cgi->a({-href=>$url}, "Sample Query 2 (second 9 matches)"));
    $column .= $cgi->p("A condition is given by parameters key1, op1, and val1, specifying the field name, comparison operator, and the (threshold) value, respectively. Currently you can specify the following fields:");
#sortに使えるキーを、各pluginに問いあわせる。
    my @s;
    push @s, $cgi->th( [ "field", "explanation" ] );
    push @s, $cgi->td( [ "id", "Graph ID" ] );
    my @keys;
    my @labels;
    foreach my $plugin ( values %{$plugins} ){
	push @keys,   $plugin->sortkey();
	push @labels, $plugin->keylabel();
    }
    while( my $key = shift @keys ){
	my $label = shift @labels;
	push @s, $cgi->td( [ $key, $label ] );
    }
    $column .= $cgi->table( $cgi->Tr( [ @s ] ) );
    
    $column .= $cgi->p("and the following operators:");
    $column .= $cgi->table( $cgi->Tr( [ $cgi->th( [ "operator", "explanation" ] ),
					$cgi->td( [ "lt", "< value" ] ),
					$cgi->td( [ "gt", "> value" ] ),
					$cgi->td( [ "eq", "== value" ] ),
					$cgi->td( [ "ne", "!= value" ] ),
					$cgi->td( [ "in", "in ranges" ] )
 ] ) );
    $column .= $cgi->p("You can also search by exact graph matching. Graph topology is specified in simple notation. Here is an example query for vitrite #8.");
    $url="$RELURL?simple=A-B-C-D-E-F-A,C-G-H-F&form=xml&field=ringset";
    $column .= $cgi->a({-href=>$url}, "Sample Query");
    
    $column .= $cgi->h2("Contact");
    $column .= $cgi->p( "Any request for new algorithms and plugins, questions, and comments are welcome." . $cgi->br .
			$cgi->a({-href=>"mailto:vitroid\@gmail.com"},
				"Masakazu Matsumoto, Okayama University")
	);
    
    $column;
}

1;
