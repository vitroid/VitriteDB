#!/usr/bin/env perl

#当面、Berkeley DBとMySQLの両方にDBを保存する。graph->idの検索は従来通りberkeley DBの命令群を使うが、逆の場合はSQLに問いあわせる。

use strict;
use CGI;
use DBI;

#local modules
use vitrite::settings;
use vitrite::make qw(make);
use vitrite::attrdb qw(attrdb_open attrdb_close attrdb_scan2 attrdb_update_attribute attrdb_insert_graph ProtectSymbols ExpandSymbols compile_ranges);
use vitrite::html qw(Header Footer to_html to_xml Query Query2 Column );
use vitrite::search qw(SearchResults);

#plugins
use vitrite::ringset;
use vitrite::ringcount;
use vitrite::eulerindex;
use vitrite::shape;
use vitrite::graph;
use vitrite::crystalviewer;
use vitrite::topovolume;
use vitrite::volume;
use vitrite::rd;
use vitrite::image;
use vitrite::isvitrite;
use vitrite::symmetry;
use vitrite::spectrum;
use vitrite::solid;





#prepare plugins
my $plugins;

foreach my $plugin ( new vitrite::ringset(),
		     new vitrite::ringcount(),
		     new vitrite::eulerindex(),
		     new vitrite::shape(),
		     new vitrite::graph(),
		     new vitrite::crystalviewer(),
		     new vitrite::topovolume(),
		     new vitrite::volume(),
		     new vitrite::rd(),
		     new vitrite::isvitrite(),
		     new vitrite::symmetry(),
		     new vitrite::spectrum(),
		     new vitrite::solid(),
		     new vitrite::image() ){
    $plugins->{$plugin->name()} = $plugin;
}




#test
#foreach my $p ( @INC ){
#    print STDERR $p, "\n";
#}


#BUILD mode. CGIではなく独立したperlスクリプトとして稼動し、データベースを一括生成する。
my $BUILD= ($ARGV[0] eq "-B")? $ARGV[1] : 0;
#TEST mode. topological indexで同一視されるフラグメントのグループを調べる。
my $TEST= $ARGV[0] eq "-T";
#my $DUMP= $ARGV[0] eq "-D";

if ( $BUILD ){
    my $db = attrdb_open( "vitrite", "matto", "vitrite" );

    my @fields = keys %{$plugins};
    my $start = $ARGV[1] || 1;
    for( my $id=$start; $id < 170000; $id++ ){
	print STDERR "$id ";
	my $record = attrdb_scan2( $db, $id, $plugins, @fields );
    }
    attrdb_close($db);
    exit 0;
}

if ( $TEST ){
    my $db = attrdb_open( "vitrite", "matto", "vitrite" );
    my @nakama;
    my $id = 1;

    #collect keys with exact values
    my @keys;
    foreach my $key ( keys %{$plugins} ){
	unless ( $plugins->{$key}{inexact} ){
	    push @keys, $key;
	}
    }
    my $keys = join(",", @keys );
    #
    #とりあえず1000までで試してみよう。
    #
    while( $id < 1000 ){
	if ( $nakama[$id] == 0 ){
	    #
	    #まず、exactな値を入手する。
	    #
	    my $sql = "SELECT $keys FROM attr WHERE id=$id";
	    my $sth = $db->prepare( $sql );
	    $sth->execute;
	    my $record = $sth->fetchrow_hashref;
	    $sth->finish;
	    #
	    #exactなトポロジー指標が完全に一致するレコードをさがす。
	    #
	    my @and;
	    foreach my $key ( keys %{$record} ){
		push @and, "$key='" . $record->{$key} ."'";
	    }
	    my $and = join( " AND ", @and );
	    my $sql = "SELECT id FROM attr WHERE $and";
	    my $sth = $db->prepare( $sql );
	    $sth->execute;
	    my $rows = $sth->rows;
	    #
	    #isospectrumなフラグメントの個数を画面表示する。
	    #
	    if ( $rows > 1 ){
		print STDERR "$id $rows\n";
	    }
	    else{
		print STDERR "$id $rows\r";
	    }
	    #
	    #エラトステネスの篩のように、配列にマークして、再計算を防ぐ。
	    #
	    while( $record = $sth->fetchrow_hashref ){
		my $pal = $record->{id};
		$nakama[$pal] = $id;
	    }
	    $sth->finish;
	}
	$id++;
    }
    attrdb_close($db);
    exit 0;
}

#if ( $DUMP ){
#    Dump( $ARGV[1] );
#}


#start CGI
my $q = new CGI;
my $db = attrdb_open( "vitrite", "matto", "vitrite" );


my $id;  #graph ID
my $record;

#
#出力すべきフィールドが限定されている場合。
#
my @fields = set_fields( $q, $plugins );
#
#collect keys with exact values
#
my @exact_fields;
foreach my $key ( keys %{$plugins} ){
    unless ( $plugins->{$key}{inexact} ){
	push @exact_fields, $key;
    }
}
my $exact_fields = join(",", @exact_fields );

#elsif ( "submit_ng" eq $q->param("q") ){
#decided not by button label but ngph contents
if ( "" ne $q->param("ngph") || "" ne $q->param("simple") ){
    my $g;
    #無向でることを確認する。
    if ( $q->param("ngph") ne "" ){
	$g = undirected( vitrite::graph::from_ngph( $q->param("ngph") ) );
    }
    if ( $q->param("simple") ne "" ){
	$g = undirected( vitrite::graph::from_simple( $q->param("simple") ) );
    }
    #グラフを試作する。
    $record->{id} = 0;
    $record->{graph} = $g;
    $record->{graph_t} = 1; #force invoke all make processes
    #スペクトルを計算する。
    vitrite::make::make( $plugins, $record, "spectrum" );
    #isospectrumなグラフを検索する。
    my $sql = "SELECT id FROM attr WHERE spectrum='" . $record->{spectrum} ."'";
    my $sth = $db->prepare( $sql );
    #print STDERR $sql, "\n";
    $sth->execute;
    my $rows = $sth->rows;
    #isospectrumなフラグメントの個数を画面表示する。
    if ( $rows == 1 ){
	#一つしかなければそれをロードする(試作グラフはすててよい)
	$id = $sth->fetchrow_hashref->{id};
	$record = attrdb_scan2( $db, $id, $plugins, @fields );
	$q->param("id", $id);
    }
    elsif ( $rows > 1 ){
	#スペクトルの性能が不十分だと、isospectrumなグラフが複数あるかもしれない。
	#ただし、その場合は、マッチしたグラフがひとつしかない場合でも、isomorphであることを保証できないので、きちんとしたnetmatchが必要になる。危険防止のためには、全数netmatchすべき。
	#netmatch
	print STDERR "Multiple Match\n";
    }
    else{
	#データベースに存在しないグラフだった場合は、ほかのトポロジー指標を計算する。	
	foreach my $key ( @exact_fields ){
	    vitrite::make::make( $plugins, $record, $key );
	}
	$q->param("id", 0);
	#もしvitrite(で、かつ巨大すぎなければ)だったなら、データベースに新規登録して番号を得たほうがいい。
    }
}
elsif ( "search by id" eq $q->param("q") || ! defined $q->param("q") ){
    #cgi paramで渡される値は常にsanitizeすべき。何がはいっているかわからない。
    $id = $q->param("id") + 0;
    #該当するidのグラフをデータベースからとりだす。指定されたフィールドが未定義ならmakeする。
    $record = attrdb_scan2( $db, $id, $plugins, @fields );
}



#
#XML形式を求められた場合。
#
if ( $q->param("form") =~ /xml/i ){
    print "Content-type: text/plain\r\n\r\n";
    if ( defined $q->param("id") ){
	#only one graph
	print to_xml( $record, $plugins, @fields ), "\n";
    }
    else{
	#a set of graphs
	print SearchResults( $q, $db, $plugins, \@fields );
    }
    attrdb_close($db);
    exit 0;
}

#
#HTML形式で出力する場合。
#

#show default form.
my $title = "Vitrite Database";
if ( $id ){
    $title = "Vitrite #$id";
}
print Header( $q, $title );

print $q->div({-class=>"panel"}, Query( $q ));
print $q->div({-class=>"panel"}, Query2( $q, $plugins ));
if ( my $html = to_html( $q, $record, $plugins ) ){
    #
    #グラフが指定されている場合は、それを出力する。
    #$recordが未定義であれば↓でリストを出力する。
    #なんとなく冗長だな。検索と出力を分離したほうがよさそう。
    #
    print $html;
}
else{
    #
    #全く何も指定されなかった場合もリストを出力する。
    #そうすることによって、はじめてアクセスした人にも、このシステムの機能がわかる。
    #
    print SearchResults( $q, $db, $plugins );
}
    

print $q->div({-class=>"column"}, Column( $q, $plugins ) );
#colophon
print $q->div({-class=>"footer"}, "Vitrite database system Copyright(c) 2007 Masakazu Matsumoto All right reserved.");

print Footer( $q );
attrdb_close($db);

exit 0;





sub undirected{
    my ( $g ) = @_;
    if ( $g ){
	$g->{directed}=0;
	my @stack;
	foreach my $x ( keys %{$g->{edges}} ){
	    foreach my $y ( keys %{$g->{edges}{$x}} ){
		push @stack, $y, $x;
	    }
	}
	while( 1<=$#stack ){
	    my $x = shift @stack;
	    my $y = shift @stack;
	    $g->{edges}{$x}{$y} = 1;
	}
    }
    $g;
}






sub set_fields{
    my ( $q, $plugins ) = @_;

    # default field
    my %field;
    $field{id}=1;
    #$field{graph}=1;
    my $field = $q->param("field");
    #formがxml以外の場合は、fieldは無視する。
    if ( $q->param("form") !~ /xml/i ){
	$field = "";
    }
    if ( $field eq "" || $field =~ /all/i ){
	foreach my $f ( keys %{$plugins} ){
	    $field{$f} = 1;
	}
    }
    else{
	my @fields = split(/,/, $field);
	foreach my $f ( @fields ){
	    if ( $plugins->{$f} ){
		$field{$f} = 1;
	    }
	}
    }
    keys %field;
}



		

0;
