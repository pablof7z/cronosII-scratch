bin_PROGRAMS = db mailbox date

INCLUDES = -DBUILDING_C2 -I.. -I$(srcdir) -I$(top_srcdir)/libmodules $(GNOME_INCLUDEDIR)

db_SOURCES = \
	db.c

db_LDADD = $(top_builddir)/libcronosII/libcronosII.a

mailbox_SOURCES = \
	mailbox.c

mailbox_LDADD = $(db_LDADD)

date_SOURCES = \
	date.c

date_LDADD = $(db_LDADD)
