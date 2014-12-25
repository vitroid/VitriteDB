package vitrite::plugin;

use strict;
use XML::Generator;



sub new {
    my ( $package ) = @_;
    my $self;
    $self->{"ancestors"} = [];
    bless $self, $package;
}

#internal format: none



sub name{"replace by the plugin name";}



sub make {
    my ( $self, $record ) = @_;
    return;
}




sub to_xml{
    my ( $self, $xmlg, $record ) = @_;
    return;
}



#一覧表示に使えるSQL field名
sub sortkey{
    return;
}



#field名
sub keylabel{
    return;
}



#xml出力に対応しているかどうか。
#ほとんどのプラグインはxml出力可能。imageはさすがに無理?でもURLを返すという手がある。
sub xmlready{ 1; }

1;
