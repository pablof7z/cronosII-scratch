noinst_LIBRARIES = libmodules.a

libmodules_a_SOURCES = \
	db.c				db.h						\
	error.c				error.h						\
						i18n.h						\
	mailbox.c			mailbox.h					\
	utils.c				utils.h					

libmodules_a_LIBADD = @LIBOBJS@

INCLUDES=-I. -I${top_srcdir} $(GNOME_INCLUDEDIR)
