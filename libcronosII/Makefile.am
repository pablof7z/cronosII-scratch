noinst_LIBRARIES = libcronosII.a

libcronosII_a_SOURCES = \
	db.c				db.h						\
	error.c				error.h						\
						i18n.h						\
	mailbox.c			mailbox.h					\
	message.c			message.h					\
	mime.c				mime.h						\
	request.c			request.h					\
	utils.c				utils.h						\
	utils-date.c		utils-date.h				\
	utils-net.c			utils-net.h

libcronosII_a_LIBADD = @LIBOBJS@

INCLUDES=-DBUILDING_C2 -I. -I${top_srcdir} $(GNOME_INCLUDEDIR)
