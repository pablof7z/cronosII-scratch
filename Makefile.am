if USE_TEST_SUITE
test_suite = test-suite
else
test_suite = 
endif

SUBDIRS = intl po libcronosII $(test_suite) src macros images pixmaps doc man plugins xml

bin_SCRIPTS = cronosII-config

plugindir = $(pkgdatadir)/plugins
plugin_DATA =

aboutdir = $(pkgdatadir)
about_DATA = about.html

cronosincludedir = $(includedir)/cronosII
cronosinclude_HEADERS = config.h

EXTRA_DIST =				\
	cronosII.spec			\
	cronosII.desktop		\
	c2addrbook.desktop		\
	cronosII-gmc.desktop	\
	c2addrbook-gmc.desktop	\
	mkpotfiles-in			\
	AUTHORS					\
	COPYING					\
	FEATURES				\
	NEWS					\
	README					\
	TODO					\
	ChangeLog				\
	$(about_DATA)

Internetdir = $(datadir)/gnome/apps/Internet
Internet_DATA = cronosII.desktop
Applicationsdir = $(datadir)/gnome/apps/Applications
Applications_DATA = c2addrbook.desktop
GMCdir = $(datadir)/mc/templates
GMC_DATA = cronosII-gmc.desktop c2addrbook-gmc.desktop

bzdist: distdir
	-chmod -R a+r $(distdir)
	$(TAR) choIf $(distdir).tar.bz2 $(distdir)
	-rm -rf $(distdir)

fulldist: distdir
	-chmod -R a+r $(distdir)
	$(TAR) choIf $(distdir).tar.bz2 $(distdir)
	echo Done with $(distdir).tar.bz2
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	echo Done with $(distdir).tar.gz
	-rm -rf $(distdir)
