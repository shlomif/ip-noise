package IP::Noise::C::Parser::Exception;

sub new
{
    my $class = shift;
    my $self = {};

    bless $self, $class;

    $self->initialize(@_);

    return $self;
}

sub initialize
{
    my $self = shift;

    while (scalar(@_))
    {
        my $arg = shift;
        if ($arg =~ /^-?(text)$/i)
        {
            my $text = shift;
            $self->{'text'} = $text;
        }
        elsif ($arg =~ /^-?(line)$/i)
        {
            my $line_num = shift;
            $self->{'line_num'} = $line_num;
        }
        elsif ($arg =~ /^-?(context)$/i)
        {
            my $context = shift;
            $self->{'context'} = $context;
        }
    }
}

1;

