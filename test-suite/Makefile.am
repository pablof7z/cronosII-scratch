bin_PROGRAMS = date db hash mailbox request pop3_test

INCLUDES = -DBUILDING_C2 -I.. -I$(srcdir) -I$(top_srcdir)/libmodules $(GNOME_INCLUDEDIR)

LDADD = $(top_builddir)/libcronosII/libcronosII.a
LDFLAGS = $(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(GTK_LIBS)

date_SOURCES = date.c
date_LDADD = $(LDADD)
date_LDFLAGS = $(LDFLAGS)

db_SOURCES = db.c
db_LDADD = $(LDADD)
db_LDFLAGS = $(LDFLAGS)

hash_SOURCES = hash.c
hash_LDADD = $(LDADD)
hash_LDFLAGS = $(LDFLAGS)

mailbox_SOURCES = mailbox.c
mailbox_LDADD = $(LDADD)
mailbox_LDFLAGS = $(LDFLAGS)

request_SOURCES = request.c
request_LDADD = $(LDADD)
request_LDFLAGS = $(LDFLAGS)

pop3_test_SOURCES = pop3_test.c
pop3_test_LDADD = $(LDADD)
pop3_test_LDFLAGS = $(LDFLAGS)