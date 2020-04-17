package IP::Noise::Text::Stream::In;

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

    my $file_handle = shift;

    $self->{'file'} = $file_handle;
    $self->{'line_num'} = 0;

    $self->read_next_line();
}

sub read_next_line
{
    my $self = shift;

    my $line = readline(*{$self->{'file'}});

    if ($line eq "")
    {
        return -1; #EOF
    }

    chomp($line);

    $line =~ s/#.*$//;

    $self->{'line_num'}++;
    
    $self->{'line'} = $line;
    # Set the ptr to 0.
    $self->{'ptr'} = 0;

    return 0;
}

sub get_line_num
{
    my $self = shift;

    return $self->{'line_num'};
}

sub peak_line
{
    my $self = shift;

    return substr($self->{'line'}, $self->{'ptr'});
}

sub increment_ptr
{
    my $self = shift;

    my $how_much = shift;

    if ($self->{'ptr'} + $how_much > length($self->{'line'}))
    {
        die "Not enough characters remaining in the line!\n"
    }
    $self->{'ptr'} += $how_much;    
}

1;

