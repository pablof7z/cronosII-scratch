DEFS=@DEFS@ -DLOCALEDIR=\"${localedir}\"
INCLUDES = -DBUILDING_C2 -I$(top_srcdir) -I$(includedir) \
	`gnome-config --cflags gnome` \
	 -DG_LOG_DOMAIN=\"CronosII\" -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	 -I../intl -I$(top_srcdir)/intl $(GNOME_INCLUDEDIR) -DDATADIR=\"${datadir}\" -DPKGDATADIR=\"${pkgdatadir}\"

if USE_UI_TEXT_EDITOR
UI_TEXT_EDITOR=ui-text-editor
else
UI_TEXT_EDITOR=
endif

bin_PROGRAMS = cronos $(UI_TEXT_EDITOR)

cronos_SOURCES = \
	c2-app.c					c2-app.h					\
	c2-main-window.c			c2-main-window.h			\
	install-wizard.c										\
	main-window.c				main-window.h				\
	main.c													\
	preferences.c											\
	object-ui-data.c			object-ui-data.h			\
	widget-composer.c			widget-composer.h			\
	widget-index.c				widget-index.h				\
	widget-html.c				widget-html.h				\
	widget-mail.c				widget-mail.h				\
	widget-message-transfer.c	widget-message-transfer.h	\
	widget-part.c				widget-part.h
	

cronos_LDADD = \
	./$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(LIBGLADE_LIBS) $(HTMLLIBS)

cronos_LDFLAGS = \
	./././$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(LIBGLADE_LIBS) $(HTMLLIBS)

ui_text_editor_SOURCES = \
	object-ui-data.c			object-ui-data.h			\
	ui-text-editor.c

ui_text_editor_LDADD = \
	./$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(LIBGLADE_LIBS) $(HTMLLIBS)

ui_text_editor_LDFLAGS = \
	./././$(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(LIBGLADE_LIBS) $(HTMLLIBS)
	
EXTRA_DIST = \
	README.FAQ			\
	xpm/mini_about.xpm	\
	xpm/mini_error.xpm	\
	xpm/new.xpm			\
	preferences-trans.h	\
	trans.h
