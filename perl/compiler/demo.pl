#!/usr/bin/perl -w

use strict;

use Gtk;

Gtk->init();

my @stages = 
    (
        {
            'id' => "always_accept",
            'name' => "Always Accept",
        },
        {
            'id' => "non_stable_delay",
            'name' => "Non-Stable Delay",
        },
        {
            'id' => "stable_delay",
            'name' => "Stable Delay",
        },
    );


my $scw = Gtk::ScrolledWindow->new(undef,undef);
$scw->set_policy('automatic', 'automatic');
$scw->show();
$scw->set_border_width(10);

my $button_box = Gtk::VBox->new(0,0);
$button_box->show();

$button_box->set_border_width(10);

$scw->add_with_viewport($button_box);

my ($i);

for($i=0;$i<scalar(@stages);$i++)
{
    my $button = Gtk::Button->new($stages[$i]->{'name'});
    $button->signal_connect('clicked' => eval { my $j = $i; sub { &button_clicked($j) } });
    $button_box->pack_start($button, 1, 1, 0);
    $button->show();
}

my $desc_text = Gtk::Text->new(undef,undef);
$desc_text->show();

my $code_text = Gtk::Text->new(undef,undef);
$code_text->show();

my $right_vbox = Gtk::VBox->new(0,0);
$right_vbox->pack_start($desc_text, 1, 1, 0);
$right_vbox->pack_start($code_text, 1, 1, 0);
$right_vbox->show();

my $main_hbox = Gtk::HBox->new(0, 0);
$main_hbox->pack_start($scw, 1, 1, 0);
$main_hbox->pack_start($right_vbox, 1, 1, 0);
$main_hbox->show();

my $window = new Gtk::Window('toplevel');
$window->set_name("IP-Noise Demo");
$window->set_uposition(20,20);
$window->set_usize(400,400);

$window->signal_connect("destroy" => \&Gtk::main_quit);
$window->signal_connect("delete_event" => \&Gtk::false);

$window->add($main_hbox);

$window->show();

sub button_clicked
{
    my $index = shift;

    my $id = $stages[$index]->{'id'};

    my $demo_file = "demo/$id.txt";
    my $desc_file = "demo/desc/$id.txt";

    if (! -f $demo_file)
    {
        die "$demo_file not found!\n";
    }
    if (! -f $desc_file)
    {
        die "$desc_file not found!\n";
    }
    #print $stages[$index]->{'id'}, "\n";
    system("perl tests/ker_translator.pl $demo_file &");
}


Gtk->main();
