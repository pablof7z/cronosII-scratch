DEFS=@DEFS@ -DLOCALEDIR=\"${localedir}\"
INCLUDES = -I$(top_srcdir) -I$(includedir) \
	`gnome-config --cflags gnome` \
	 -DG_LOG_DOMAIN=\"CronosII\" -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	 -I../intl -I$(top_srcdir)/intl $(GNOME_INCLUDEDIR) -DDATADIR=\"${datadir}\"

bin_PROGRAMS = cronos

cronos_SOURCES = \
	c2-app.c					c2-app.h					\
	install-wizard.c										\
	main-window.c				main-window.h				\
	main.c										
	

cronos_LDADD = \
	$(top_builddir)/libmodules/libmodules.a \
	$(LIBS) $(INTLLIBS)

cronos_LDFLAGS = \
	$(top_builddir)/libmodules/libmodules.a \
	$(LIBS) $(INTLLIBS)
