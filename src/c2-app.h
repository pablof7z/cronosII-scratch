#ifndef __C2_APP_H__
#define __C2_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#	include <config.h>
#	include <libmodules/mailbox.h>
#else
#	include <cronosII.h>
#endif
#include <gnome.h>

#define MAILBOX_INBOX		N_("Inbox"	)
#define MAILBOX_OUTBOX		N_("Outbox"	)
#define MAILBOX_QUEUE		N_("Queue"	)
#define MAILBOX_GARBAGE		N_("Garbage")
#define MAILBOX_DRAFTS		N_("Drafts"	)

#ifdef USE_DEBUG
#	define L				g_print ("%s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
#	define DEBUG(x)			g_print ("%s:%d:%s:%s: '%s'\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, #x, x);
#endif

typedef enum {
	C2_DEFAULT_MIME_PLAIN,
	C2_DEFAULT_MIME_HTML
} C2DefaultMimeType;

typedef enum {
	C2_INIT_ADDRBOOK_AT_START,
	C2_INIT_ADDRBOOK_AT_REQ,
	C2_INIT_ADDRBOOK_AT_OPEN
} C2InitAddrbookTime;

typedef enum {
	C2_MIME_WINDOW_STICKY,
	C2_MIME_WINDOW_AUTOMATIC
} C2MimeWindowMode;

typedef enum {
	SHOWABLE_HEADER_FIELD_TO	= 1 << 0,
	SHOWABLE_HEADER_FIELD_DATE	= 1 << 1,
	SHOWABLE_HEADER_FIELD_FROM	= 1 << 2,
	SHOWABLE_HEADER_FIELD_SUBJECT	= 1 << 3,
	SHOWABLE_HEADER_FIELD_ACCOUNT	= 1 << 4,
	SHOWABLE_HEADER_FIELD_CC	= 1 << 5,
	SHOWABLE_HEADER_FIELD_BCC	= 1 << 6,
	SHOWABLE_HEADER_FIELD_PRIORITY	= 1 << 7
} C2ShowableHeaderField;

enum {
	C2_SHOWABLE_HEADERS_PREVIEW,
	C2_SHOWABLE_HEADERS_MESSAGE,
	C2_SHOWABLE_HEADERS_COMPOSE,
	C2_SHOWABLE_HEADERS_SAVE,
	C2_SHOWABLE_HEADERS_PRINT,
	C2_SHOWABLE_HEADERS_LAST
};

struct C2Application {
	GtkTooltips *tooltips;

	GdkBitmap *mask_unread, *mask_read, *mask_reply, *mask_forward;
	GdkPixmap *pixmap_unread, *pixmap_read, *pixmap_reply, *pixmap_forward;
	GdkCursor *cursor_busy;
	GdkCursor *cursor_normal;
	GdkFont *gdk_font_body;
	GdkFont *gdk_font_read;
	GdkFont *gdk_font_unread;

	C2Mailbox *mailboxes;
#if FALSE
	C2Account *accounts;
	C2Modules *modules;
#endif
	C2DefaultMimeType default_mime;
	C2InitAddrbookTime addrbook_init;

	gboolean empty_garbage;
	gboolean check_at_start;
	gboolean use_outbox;
	gboolean use_persistent_smtp;
	
	gint check_timeout;
	gint message_size_limit;
	gint net_timeout;
	gint mark_as_read_timeout;
	gint persistent_smtp_port;
	
	gchar *prepend_character;
	gchar *persistent_smtp_host;

	GdkColor color_reply_original_message;
	GdkColor color_misc_body;



	C2MimeWindowMode mime_window;
	GtkToolbarStyle toolbar;

	gint wm_hpan;
	gint wm_vpan;
	gint wm_clist[8];
	gint wm_width;
	gint wm_height;
	
	guint showable_headers[C2_SHOWABLE_HEADERS_LAST];

	gchar *font_read;
	gchar *font_unread;
	gchar *font_body;
	gchar *app_title;
} c2_app;

gint
c2_app_init											(void);

void
c2_install_new										(void);

void
c2_mailbox_tree_fill								(C2Mailbox *head, GtkCTreeNode *node,
													 GtkWidget *ctree, GtkWidget *window);

#ifdef __cplusplus
}
#endif

#endif
