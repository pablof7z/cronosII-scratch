DEFS=@DEFS@ -DLOCALEDIR=\"${localedir}\"
INCLUDES = -DBUILDING_C2 -I$(top_srcdir) -I$(includedir) \
	`gnome-config --cflags gnome` \
	 -DG_LOG_DOMAIN=\"CronosII\" -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	 -I../intl -I$(top_srcdir)/intl $(GNOME_INCLUDEDIR) -DDATADIR=\"${datadir}\" -DPKGDATADIR=\"${pkgdatadir}\"

bin_PROGRAMS = cronos

cronos_SOURCES = \
	c2-app.c					c2-app.h					\
	c2-main-window.c			c2-main-window.h			\
	install-wizard.c										\
	main-window.c				main-window.h				\
	main.c													\
	preferences.c											\
	widget-index.c				widget-index.h				\
	widget-html.c				widget-html.h				\
	widget-mail.c				widget-mail.h				\
	widget-message-transfer.c	widget-message-transfer.h	\
	widget-part.c				widget-part.h				\
	widget-xtext.c				widget-xtext.h
	

cronos_LDADD = \
	$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(LIBGLADE_LIBS) $(HTMLLIBS)

cronos_LDFLAGS = \
	$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(LIBGLADE_LIBS) $(HTMLLIBS)

EXTRA_DIST = \
	README.FAQ			\
	xpm/forward.xpm		\
	xpm/mark.xpm		\
	xpm/mini_about.xpm	\
	xpm/mini_error.xpm	\
	xpm/new.xpm			\
	xpm/read.xpm		\
	xpm/reply.xpm		\
	xpm/unread.xpm
