bin_PROGRAMS = db mailbox

INCLUDES = -I.. -I$(srcdir) -I$(top_srcdir)/libmodules $(GNOME_INCLUDEDIR)

db_SOURCES = \
	db.c

db_LDADD = $(top_builddir)/libmodules/libmodules.a

mailbox_SOURCES = \
	mailbox.c

mailbox_LDADD = $(db_LDADD)
