
D = /var/www/html/ip-noise

IMAGES = $(D)/style.css \
         $(D)/docs/Arbitrator_Architecutre.txt                \
         $(D)/docs/ip_noise.spec.zip                          \
         $(D)/docs/Syntax_Description.txt                     \
         $(D)/docs/Architecture.txt                           \
         $(D)/docs/Protocol.txt                               \
         $(D)/style.css                                       \
         $(D)/download/IP-Noise-Engine-perl-0.1.4.tar.gz      \
         $(D)/download/IP-Noise-Engine-perl-0.1.1.tar.gz      \
         $(D)/download/IP-Noise-perl-filters-0.1.0.tar.gz     \
         $(D)/download/IP-Noise-Engine-perl-0.1.5.tar.gz      \
         $(D)/download/IP-Noise-Engine-perl-0.1.0.tar.gz      \
         $(D)/download/IP-Noise-Engine-perl-0.1.2.tar.gz


SUBDIRS = $(D)/download $(D)/docs $(D)/mailing-list

HTMLS = $(D)/index.html $(D)/download/index.html $(D)/links.html \
	$(D)/docs/index.html $(D)/mailing-list/index.html

all : $(SUBDIRS) $(HTMLS) $(IMAGES)

$(SUBDIRS) :: % : 
	@if [ ! -e $@ ] ; then \
		mkdir $@ ; \
	fi
	

$(HTMLS) :: /var/www/html/ip-noise/% : %.wml .wmlrc
	wml $< > $@

$(IMAGES) :: /var/www/html/ip-noise/% : %
	cp -f $< $@
