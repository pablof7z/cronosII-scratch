bin_PROGRAMS = index_is_clean

INCLUDES = -DBUILDING_C2 -I.. -I$(srcdir) -I$(top_srcdir)/libmodules -I$(top_srcdir)/src $(GNOME_INCLUDEDIR)

LDADD = $(top_builddir)/libcronosII/libcronosII.a
LDFLAGS = $(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(GTK_LIBS) $(HTMLLIBS) $(GCONFLIBS) -pthread

index_is_clean_SOURCES = index_is_clean.c
index_is_clean_LDADD = $(LDADD)
index_is_clean_LDFLAGS = $(LDFLAGS)
