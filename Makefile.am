if COMPILE_MODULE_TEST
test_modules = module-test
else
test_modules =
endif

SUBDIRS = intl po libmodules $(test_modules) src images pixmaps doc man plugins

bin_SCRIPTS = cronosII-config

plugindir = $(pkgdatadir)/plugins
plugin_DATA =

cronosincludedir = $(includedir)/cronosII
cronosinclude_HEADERS = config.h

EXTRA_DIST =			\
	cronosII.spec		\
	cronosII.desktop	\
	c2addrbook.desktop	\
	cronosII-gmc.desktop	\
	c2addrbook-gmc.desktop	\
	cronosII.spec		\
	AUTHORS			\
	COPYING			\
	FEATURES		\
	NEWS			\
	README			\
	TODO			\
	ChangeLog

Internetdir = $(datadir)/gnome/apps/Internet
Internet_DATA = cronosII.desktop
Applicationsdir = $(datadir)/gnome/apps/Applications
Applications_DATA = c2addrbook.desktop
GMCdir = $(datadir)/mc/templates
GMC_DATA = cronosII-gmc.desktop c2addrbook-gmc.desktop
