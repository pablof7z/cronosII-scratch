noinst_LIBRARIES = libcronosII.a

libcronosII_a_SOURCES = \
	account.c			account.h					\
	db.c				db.h						\
	db-cronosII.c		db-cronosII.h				\
	db-imap.c			db-imap.h					\
	db-spool.c			db-spool.h					\
	error.c				error.h						\
						i18n.h						\
	mailbox.c			mailbox.h					\
	message.c			message.h					\
	mime.c				mime.h						\
	net-object.c		net-object.h				\
	pop3.c				pop3.h						\
	request.c			request.h					\
	smtp.c				smtp.h						\
	utils.c				utils.h						\
	utils-date.c		utils-date.h				\
	utils-net.c			utils-net.h

libcronosII_a_LIBADD = @LIBOBJS@

INCLUDES=-DBUILDING_C2 -I. -I${top_srcdir} $(GNOME_INCLUDEDIR)

EXTRA_DIST = \
	README				\
	README.api			\
	README.mailbox_id	
