package vitrite::search;

use strict;
use CGI;
use XML::Generator;
use vitrite::attrdb qw (compile_ranges ExpandSymbols);
use vitrite::html   qw (to_xml );
BEGIN{
    use Exporter ();
    use vars qw(@ISA @EXPORT @EXPORT_OK);
    
    @ISA       = qw(Exporter);
    @EXPORT    = qw( SearchResults );
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



#検索条件はcgi paramにて与えられる。
#cgi paramをSQLに翻訳し、検索し、結果をリスト表示する。
#これ以前の部分は1つのフラグメントの表示として完結しているので、
#このルーチンは別モジュールに分けるべきだろう。
sub SearchResults{
    my ( $cgi, $db, $plugins, $inxml ) = @_;
    #print STDERR "SearchResults\n";
    #不正な条件が与えられた時はNULLを返す。
    
    # sta: start from
    # rec: number of records to be shown in one screen
    # rev: reverse order
    # srt: sort by
    # まずは条件なし。id順に出力する。

    #同じページを表示するための、CGIパラメータ群
    my %p;

    #sortに使えるキーを、各pluginに問いあわせる。
    my @keys;
    my %keys;
    foreach my $plugin ( values %{$plugins} ){
	push @keys, $plugin->sortkey();
    }
    foreach my $sortkeys ( @keys ){
	$keys{$sortkeys} = 1;
    }
    @keys = ( "id", "image", @keys );
    #キーの名称を、各pluginに問いあわせる。
    my @labels = ("ID","image");
    foreach my $plugin ( values %{$plugins} ){
	push @labels, $plugin->keylabel();
    }

    #表示順序の指定。
    my $reverse = $cgi->param("rev") + 0; # ? "DESC" : "ASC";
    my $asc = $reverse ? "DESC" : "ASC";

    my $sort = "id";
    if ( $cgi->param("srt") ){
	$sort= $cgi->param("srt");
	return if ( $sort ne "id" && ! $keys{$sort} );
    }


    #区間の処理。
    my $start = $cgi->param("sta") + 0;

    my $records = $cgi->param("rec") + 0;
    if ( $cgi->param("form") =~ /xml/i ){
	#
	#指定がなければ1000個.
	#still, it is unlimited if number is specified.
	#It is fail-safe for bad query.
	#
	if ( $records == 0 ){
	    $records = 1000;
	}
    }
    else{
	#
	#指定がなければ50個表示
	#
	if ( $records == 0 || 50 < $records ){
	    $records = 50;
	}
    }
    my $limit;
    #無制限の場合はstartも無視する。
    if ( 0 < $records ){
	$limit = "LIMIT $start, $records";
    }

    #条件の読み込み。
    my %operators = ( "eq"=>"=",
		      "ne"=>"!=",
		      "lt"=>"<",
		      "gt"=>">",
		      "in"=>"in"
	);
    my @cond;
    foreach my $x ( 1..5 ){
	if ( "" ne $cgi->param("val$x") ){
	    #parameterで与えられた文字列をそのままSQLに渡すとsecurity holeになる。
	    my $valx = $cgi->param("val$x");
	    my $keyx = $cgi->param("key$x");
	    my $opx  = $cgi->param("op$x");
	    return "" unless defined $operators{$opx};
	    return "" unless ( $keyx eq "id" || $keys{$keyx} );
	    if ( $opx eq "in" ){
		push @cond, compile_ranges( $keyx, $valx );
	    }
	    else{ 
		$valx += 0; #sanitize
		push @cond ,  $keyx . $operators{$opx} . $valx;
	    }
	    #formを介さずに検索を行う場合に%pを使用する。
	    $p{"key$x"} = $keyx;
	    $p{"op$x"}  = $opx;
	    $p{"val$x"} = $valx;
	}
    }

    my $cond;
    if ( 0<=$#cond ){
	$cond = "WHERE " . join( " AND ", @cond );
    }

    my $sql;
    my $keys = join(",", @keys );
    if ( $inxml ){
	#xmlの場合は、指定されたfieldのみを抽出する。
	$keys = join(",", @{$inxml});
    }
    #get number of matches in advance
    $sql = "SELECT id FROM attr $cond";
    #print STDERR "$sql SearchResults\n";
    my $sth = $db->prepare( $sql );
    $sth->execute;
    my $rows = $sth->rows;
    $sth->finish;

    $sql = "SELECT $keys FROM attr $cond ORDER BY $sort $asc $limit";
    #print STDERR "$sql DEBUG\n";
    my $sth = $db->prepare( $sql );
    $sth->execute;

    $p{sta} = $start;
    $p{rec} = $records;
    $p{srt} = $sort;
    $p{rev} = $reverse;
    $p{q} =   "Search"; #dummy data for test

    if ( $inxml ){
	#$inxmlは実はxml出力すべきフィールドの配列への参照
	my @s;
	#compose values
	while ( my $record = $sth->fetchrow_hashref ){
	    foreach my $key ( keys %{$record} ){
		if ( defined $record->{$key} ){
		    my $value = ExpandSymbols( $record->{$key} );
		    if ( defined $plugins->{$key} ){
			$value = $plugins->{$key}->from_sql( $value );
		    }
		    $record->{$key} = $value;
		}
	    }
	    push @s, to_xml( $record, $plugins, @{$inxml} );
	}
	$sth->finish;
	my $xml = new XML::Generator;
	return $xml->vitrites( @s );
    }
    else{
	my $html;

	#link to the next page
	my $nextpage;
	if ( $records && $start + $records < $rows ){
	    my %newp = %p;
	    $newp{sta} = $start + $records;
	    $nextpage = $cgi->a({-href=>to_cgiparam( %newp )}, "| next $records >");
	}
	#link to the prev page
	my $prevpage;
	if ( $records && 0 < $start ){
	    my %newp = %p;
	    my $newstart = $start - $records;
	    if ( $newstart < 0 ){
		$newstart = 0;
	    }
	    $newp{sta} = $newstart;
	    $prevpage = $cgi->a({-href=>to_cgiparam( %newp )}, "< prev $records |");
	}
	#change sort key
	my @keysort;
	foreach my $key ( @keys ){
	    my %newp = %p;
	    if ( $sort eq $key ){
		#逆ソートにする。
		$newp{rev} = 1-$newp{rev};
	    }
	    else{
		#それを新しいソートキーにする
		$newp{srt} = $key;
		#print STDERR "SORT KEY $sort -> $key\n";
	    }
	    if ( $key eq "image" ){
		push @keysort, ( shift @labels );
	    }
	    else{
		push @keysort, $cgi->a({-href=>to_cgiparam( %newp )}, shift @labels );
	    }
	}
	
	#compose labels
	$html .= $cgi->Tr({-class=>"gray"}, $cgi->th({-class=>"search"}, [  @keysort  ] ) );
	
	#compose values
	my @results;
	my $even = 1;
	while ( my @array = $sth->fetchrow_array ){
	    #special process for image
	    if ( $array[1] ){
		my $id = $array[0];
		$array[1] = $cgi->a({-href=>"$RELURL?id=$id"}, $cgi->img({-src=>"$IMAGEURL/$array[1].png",-width}));
	    }
	    if ( $array[0] ){
		my $id = $array[0];
		$array[0] = $cgi->a({-href=>"$RELURL?id=$id"}, $id);
	    }
	    my $class = $even ? "white" : "gray";
	    push @results, $cgi->Tr({-class=>$class}, $cgi->td({-class=>"search"}, [  @array  ] ) );
	    $even = ! $even;
	}
	$html .= join( "", @results );
	
	$sth->finish;
	$html = $cgi->h1("Search Results")
	    .   $cgi->p("$prevpage $rows total $nextpage")
	    .   $cgi->table({-class=>"search"}, $html )
	    .   $cgi->p("$prevpage $rows total $nextpage");
	return $html;
    }
}



sub to_cgiparam{
    my ( %hash ) = @_;
    my @params;
    foreach my $key ( keys %hash ){
	push @params, "$key=" . $hash{$key};
    }
    return "$RELURL?" . join("&", @params );
}



1;
