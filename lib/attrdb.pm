package vitrite::attrdb;

use strict;
use vitrite::make qw(make);


BEGIN{
    use Exporter ();
    use vars qw(@ISA @EXPORT @EXPORT_OK);
    
    @ISA       = qw(Exporter);
    @EXPORT    = qw(attrdb_open attrdb_close attrdb_scan2 attrdb_update_attribute attrdb_insert_graph ProtectSymbols ExpandSymbols compile_ranges);
    @EXPORT_OK = qw();
}

use vars @EXPORT_OK;




#for access to attribute DB
sub attrdb_open{
    my ( $name, $user, $password ) = @_;

    my $dsn = "DBI:mysql:$name:localhost";

    my $db;
    for( my $i=0; $i<2; $i++ ){
	$db = DBI->connect($dsn, $user, $password);
	last if $db;
	sleep 3;
    }
    die if ! $db;
    $db->{AutoCommit} = 0; #enable transaction if possible
    $db->{RaiseError} = 1;
    $db;
}



sub attrdb_close{
    my ( $db ) = @_;

    $db->disconnect;
}




sub attrdb_scan2{
    my ( $db, $id, $plugins, @fields ) = @_;

    die if $id !~ /^[0-9]*$/;
    #読めるレコードは全部読みだす。
    my $sql = "SELECT * FROM attr WHERE id='$id'";
    my $sth = $db->prepare( $sql );
    $sth->execute;
    my $rows = $sth->rows;
    my $record;
    if ( $rows == 1 ){
	$record = $sth->fetchrow_hashref;
	foreach my $key ( keys %{$record} ){
	    if ( defined $record->{$key} ){
		my $value = ExpandSymbols( $record->{$key} );
		if ( defined $plugins->{$key} ){
		    $value = $plugins->{$key}->from_sql( $value );
		}
		$record->{$key} = $value;
	    }
	    else{
	    }
	}
    }
    $sth->finish;

    #make attributes
    if ( $record ){
	$record->{save} = [];
	foreach my $field ( @fields ){
	    #print STDERR $record->{isvitrite}, ":attrdb.pm+80\n";
	    make( $plugins, $record, $field );
	    #print STDERR $record->{isvitrite}, ":attrdb.pm+81\n";
	}
	my $sql = join(",", @{$record->{save}});
	if ( $sql ne "" ){
	    #print STDERR "SET $sql\n";
	    attrdb_update_attribute( $db, $id, $sql );
	}
	#print STDERR $record->{isvitrite}, ":attrdb.pm+89\n";
    }
		
    $record;
}



sub attrdb_update_attribute{
    my ( $db, $id, $sql ) = @_;
    die if $id !~ /^[0-9]*$/;
    #$sql = ProtectSymbols( $sql );
    #values must be properly protected.
    $sql = "UPDATE attr SET $sql WHERE id='$id'";
    $db->do( $sql );
}






sub attrdb_insert_graph{
    my ( $db, $id, $g ) = @_;

    my $str = vitrite::graph::to_ngph( $g );
    my @setstring;
    push( @setstring, "graph='".ProtectSymbols( $str )."'");

    if ( $id == 0 ){
	die "ID unspec.\n"; #今のところ新規登録は禁止。
	#
	#make new record
	#
	my $sql = "INSERT attr SET " . join( ", ", @setstring );
	$db->do( $sql );
	#
	#get the record
	#
	$sql = "SELECT LAST_INSERT_ID()";
	my $sth = $db->prepare($sql);
	$sth->execute;
	my $rows = $sth->rows;
	my $record = $sth->fetchrow_array;
	$sth->finish;
	#print STDERR "LAST: $record\n";
	return $record;
    }
    else{
	my $sql = "INSERT attr SET " . join( ", ", @setstring );
	$sql .= ", id=$id";
	#print STDERR $sql, "\n";
	$db->do( $sql );
	#print STDERR $id, "\n";
	return $id;
    }
}



sub ProtectSymbols{
    my ($text)=@_;
    #$text=~ s/([^0-9A-Za-z_])/"%".unpack("H2",$1)/gexi;
    #$text=~ s/([\'\%\\])/"%".unpack("H2",$1)/gexi;
    $text=~ s/\n/\\n/g;
    $text=~ s/\\/\\\\/g;
    $text=~ s/\'/\\'/g;
    $text=~ s/\0/\\0/g;
    return $text;
}



sub ExpandSymbols{
    my ($text)=@_;
    #$text=~ s/\%([0-9a-f]{2,2})/pack("H2",$1)/gexi;
    $text=~ s/\\n/\n/g;
    return $text;
}



#compile input string as the range specifications
#convert to SQL expressions.
sub compile_ranges{
    my ( $tag, $s ) = @_;
    my @ranges = split(/[\s,]+/, $s);
    my @or;
    foreach my $range ( @ranges ){
	if ( $range =~ /^[0-9]+$/ ){
	    push @or, "$tag=$range";
	}
	elsif ( $range =~ /^([0-9]*)[-:]([0-9]*)$/ ){
	    my ( $from, $to ) = ( $1, $2 );
	    my @and;
	    if ( $from ne "" ){
		push @and, "$tag>=$from";
	    }
	    if ( $to ne "" ){
		push @and, "$tag<=$to";
	    }
	    push @or, "(" . join( " AND ", @and ) . ")";
	}
    }
    my $sql = "(" . join( " OR ", @or ) . ")";
    #print STDERR $sql, " RANGE\n";
    return $sql;
}




1;
