if USE_TEST_SUITE
test_suite = test-suite
else
test_suite = 
endif

if USE_TOOLS
tools = tools
else
tools =
endif

SUBDIRS = intl po libcronosII src $(test_suite) $(tools) macros default images pixmaps doc man plugins xml

bin_SCRIPTS = cronosII-config

plugindir = $(pkgdatadir)/plugins
plugin_DATA =

aboutdir = $(pkgdatadir)
about_DATA = about.html bugs.html features.html header.png message_404.html touch.html welcome.html

desktopdir = $(prefix)/gnome/apps/Internet
desktop_DATA = cronosII.desktop

cronosincludedir = $(includedir)/cronosII
cronosinclude_HEADERS = config.h

EXTRA_DIST =				\
	cronosII.spec			\
	mkpotfiles-in			\
	ABOUT-NLS				\
	AUTHORS					\
	COPYING					\
	FEATURES				\
	NEWS					\
	README					\
	README.1st				\
	THANKS					\
	TODO					\
	ChangeLog				\
	$(about_DATA)

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
