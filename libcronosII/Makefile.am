noinst_LIBRARIES = libcronosII.a

libcronosII_a_SOURCES = \
	date-utils.c		date-utils.h				\
	db.c				db.h						\
	error.c				error.h						\
						i18n.h						\
	mailbox.c			mailbox.h					\
	message.c			message.h					\
	utils.c				utils.h					

libcronosII_a_LIBADD = @LIBOBJS@

INCLUDES=-DBUILDING_C2 -I. -I${top_srcdir} $(GNOME_INCLUDEDIR)
