package vitrite::make;

use strict;

BEGIN{
    use Exporter ();
    use vars qw(@ISA @EXPORT @EXPORT_OK);
    
    @ISA       = qw(Exporter);
    @EXPORT    = qw(make);
    @EXPORT_OK = qw();
}

use vars @EXPORT_OK;


#階層的に呼びだした中で、最も新しい時刻を返す。
sub make{
    my ( $plugins, $record, $attr, $depth ) = @_;

    my $last = 0;
    #print STDERR ". "x$depth , $record->{isvitrite},":make.pm+22($attr)\n";
    #return if ( "" ne $record->{$attr} );
    return $last if ( ! defined $plugins->{$attr} );
    #print STDERR "start making $attr.\n";

    #make ancestors in advance.
    foreach my $ancestor ( @{$plugins->{$attr}{"ancestors"}} ){
	my $t = make( $plugins, $record, $ancestor, $depth+1 );
	if ( $last < $t ){
	    $last = $t;
	}
    }
    #再度チェック。
    #return if ( "" ne $record->{$attr} );
    #自分の時刻がancestorのどれかより古ければmakeする。
    #print STDERR ". "x$depth , $record->{$attr."_t"},":make.pm+37($attr)\n";
    if ( $record->{$attr."_t"} < $last ){
	my $result = $plugins->{$attr}->make( $record );
	#print STDERR "make.pm+39($result)\n";
	if ( defined $result ){
	    #update the database
	    #print STDERR " $attr ATTR2\n";
	    #{save} holds list of updated fields.
	    push @{$record->{save}}, $plugins->{$attr}->to_sql( $result );
	    #added
	    my $now = time;
	    push @{$record->{save}}, $attr."_t=$now";
	    $record->{$attr."_t"} = $now;
	    return $now;
	}
    }
    #自分のほうが新しいわけだから自分の時刻を返す。
    return $record->{$attr."_t"};
}



1;
