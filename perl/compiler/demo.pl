#!/usr/bin/perl -w

use strict;

use Gtk::Gdk;
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
    $button_box->pack_start($button, 0, 1, 0);
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

my $window = new Gtk::Window('toplevel');

my $main_hbox = Gtk::HBox->new(0, 0);
#my $geo = Gtk::Gdk::Geometry->new(50,50,50,50);
#$window->set_geometry_hints($scw, { 'max_width' => 100 }, 0);
$scw->set_usize(130,0);
$main_hbox->pack_start($scw, 0, 1, 0);
$main_hbox->pack_start($right_vbox, 1, 1, 0);
$main_hbox->show();


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

    open I, "<$demo_file";
    my $demo_contents = join("",<I>);
    close(I);

    open I, "<$desc_file";
    my $desc_contents = join("",<I>);
    close(I);

    my $font = Gtk::Gdk::Font->load("-adobe-courier-medium-r-normal-*-*-140-*-*-m-*-iso8859-1");

    my $set_text = sub {
        my $widget = shift;
        my $contents = shift;

        $widget->freeze();
        $widget->realize();
        $widget->set_point(0);
        $widget->forward_delete($widget->get_length());
        $widget->insert($font, $widget->style->black, undef, $contents);

        $widget->thaw();        
    };

    $set_text->($desc_text, $desc_contents);
    $set_text->($code_text, $demo_contents);
   

}


Gtk->main();

