bin_PROGRAMS = account date db hash mailbox message pop3 request smtp

INCLUDES = -DBUILDING_C2 -I.. -I$(srcdir) -I$(top_srcdir)/libmodules $(GNOME_INCLUDEDIR)

LDADD = $(top_builddir)/libcronosII/libcronosII.a
LDFLAGS = $(top_builddir)/libcronosII/libcronosII.a \
	$(LIBS) $(INTLLIBS) $(GNOME_LIBDIR) $(GNOMEUI_LIBDIR) $(PTHREAD_LIB) \
	$(GNOME_PRINT_LIBS) $(GLIB_LIBS) $(GTK_LIBS)

account_SOURCES = account.c
account_LDADD = $(LDADD)
account_LDFLAGS = $(LDFLAGS)

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

message_SOURCES = message.c
message_LDADD = $(LDADD)
message_LDFLAGS = $(LDFLAGS)

pop3_SOURCES = pop3.c
pop3_LDADD = $(LDADD)
pop3_LDFLAGS = $(LDFLAGS)

request_SOURCES = request.c
request_LDADD = $(LDADD)
request_LDFLAGS = $(LDFLAGS)

smtp_SOURCES = smtp.c
smtp_LDADD = $(LDADD)
smtp_LDFLAGS = $(LDFLAGS)
