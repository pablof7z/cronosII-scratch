bin_PROGRAMS = db mailbox date

INCLUDES = -DBUILDING_C2 -I.. -I$(srcdir) -I$(top_srcdir)/libmodules $(GNOME_INCLUDEDIR)

db_SOURCES = \
	db.c

db_LDADD = $(top_builddir)/libcronosII/libcronosII.a
db_LDFLAGS = $(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS)

mailbox_SOURCES = \
	mailbox.c

mailbox_LDADD = $(db_LDADD)
mailbox_LDFLAGS = $(db_LDFLAGS)

date_SOURCES = \
	date.c

date_LDADD = $(db_LDADD)
date_LDFLAGS = $(db_LDFLAGS)
