bin_PROGRAMS = db mailbox date request

INCLUDES = -DBUILDING_C2 -I.. -I$(srcdir) -I$(top_srcdir)/libmodules $(GNOME_INCLUDEDIR)

LDADD = $(top_builddir)/libcronosII/libcronosII.a
LDFLAGS = $(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS)

db_SOURCES = db.c
db_LDADD = $(LDADD)
db_LDFLAGS = $(LDFLAGS)

mailbox_SOURCES = mailbox.c
mailbox_LDADD = $(LDADD)
mailbox_LDFLAGS = $(LDFLAGS)

date_SOURCES = date.c
date_LDADD = $(LDADD)
date_LDFLAGS = $(LDFLAGS)

request_SOURCES = request.c
request_LDADD = $(LDADD)
request_LDFLAGS = $(LDFLAGS)
