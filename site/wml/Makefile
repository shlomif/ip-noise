
D = /var/www/html/ip-noise
#D = /home/httpd/html/ip-noise

IMAGES = $(D)/style.css                                             \
         $(D)/docs/Arbitrator_Architecture.txt                      \
         $(D)/docs/Architecture.txt                                 \
         $(D)/docs/ip_noise.spec.zip                                \
         $(D)/docs/Mid-Term.ppt                                     \
         $(D)/docs/Protocol.txt                                     \
         $(D)/docs/Syntax_Description.txt                           \
         $(D)/docs/roadmaps/2001-08-22.txt                          \
         $(D)/docs/roadmaps/2001-09-11.txt                          \
         $(D)/download/IP-Noise-C-Arbitrator-0.1.1.tar.gz           \
         $(D)/download/IP-Noise-C-Arbitrator-0.1.2.tar.gz           \
         $(D)/download/IP-Noise-Compiler-2001-08-28-rev1.tar.gz     \
         $(D)/download/IP-Noise-Compiler-0.1.2.tar.gz               \
         $(D)/download/IP-Noise-Compiler-0.1.3.tar.gz               \
         $(D)/download/IP-Noise-Compiler-0.1.4.tar.gz               \
         $(D)/download/IP-Noise-Engine-perl-0.1.0.tar.gz            \
         $(D)/download/IP-Noise-Engine-perl-0.1.1.tar.gz            \
         $(D)/download/IP-Noise-Engine-perl-0.1.2.tar.gz            \
         $(D)/download/IP-Noise-Engine-perl-0.1.4.tar.gz            \
         $(D)/download/IP-Noise-Engine-perl-0.1.5.tar.gz            \
         $(D)/download/IP-Noise-Perl-Arbitrator-0.1.2.tar.gz        \
         $(D)/download/IP-Noise-Perl-Arbitrator-0.1.3.tar.gz        \
         $(D)/download/IP-Noise-Perl-Arbitrator-0.1.4.tar.gz        \
         $(D)/download/IP-Noise-Perl-Arbitrator-0.1.5.tar.gz        \
         $(D)/download/IP-Noise-perl-filters-0.1.0.tar.gz           \
         


SUBDIRS = $(D)/download $(D)/docs $(D)/docs/roadmaps $(D)/mailing-list

HTMLS = $(D)/index.html $(D)/berlios.html $(D)/download/index.html $(D)/links.html \
	$(D)/docs/index.html $(D)/mailing-list/index.html

all : $(SUBDIRS) $(HTMLS) $(IMAGES)

$(SUBDIRS) :: % : 
	@if [ ! -e $@ ] ; then \
		mkdir $@ ; \
	fi
	

$(HTMLS) :: $(D)/% : %.wml .wmlrc template.wml
	wml $< > $@

$(IMAGES) :: $(D)/% : %
	cp -f $< $@
