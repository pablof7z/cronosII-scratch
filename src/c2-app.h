/*  Cronos II
 *  Copyright (C) 2000-2001 Pablo Fernández Navarro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __C2_APP_H__
#define __C2_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#	include <libcronosII/mailbox.h>
#else
#	include <cronosII.h>
#endif
#include <gnome.h>

#define MAILBOX_INBOX		_("Inbox"	)
#define MAILBOX_OUTBOX		_("Outbox"	)
#define MAILBOX_QUEUE		_("Queue"	)
#define MAILBOX_GARBAGE		_("Garbage"	)
#define MAILBOX_DRAFTS		_("Drafts"	)

#define DATE_FORMAT								"%d.%m.%Y %H:%M:%S %z"

#ifdef USE_DEBUG
#	define L				g_print ("%s:%d:%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
#	define DEBUG(x)			g_print ("%s:%d:%s:%s: '%s'\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, #x, x);
#endif

typedef enum
{
	C2_DEFAULT_MIME_PLAIN,
	C2_DEFAULT_MIME_HTML
} C2DefaultMimeType;

typedef enum
{
	C2_INIT_ADDRBOOK_AT_START,
	C2_INIT_ADDRBOOK_AT_REQ,
	C2_INIT_ADDRBOOK_AT_OPEN
} C2InitAddrbookTime;

typedef enum
{
	C2_MIME_WINDOW_STICKY,
	C2_MIME_WINDOW_AUTOMATIC
} C2MimeWindowMode;

typedef enum
{
	C2_SHOWABLE_HEADER_FIELD_TO			= 1 << 0,
	C2_SHOWABLE_HEADER_FIELD_DATE		= 1 << 1,
	C2_SHOWABLE_HEADER_FIELD_FROM		= 1 << 2,
	C2_SHOWABLE_HEADER_FIELD_SUBJECT	= 1 << 3,
	C2_SHOWABLE_HEADER_FIELD_ACCOUNT	= 1 << 4,
	C2_SHOWABLE_HEADER_FIELD_CC			= 1 << 5,
	C2_SHOWABLE_HEADER_FIELD_BCC		= 1 << 6,
	C2_SHOWABLE_HEADER_FIELD_PRIORITY	= 1 << 7
} C2ShowableHeaderField;

enum
{
	C2_SHOWABLE_HEADERS_PREVIEW,
	C2_SHOWABLE_HEADERS_MESSAGE,
	C2_SHOWABLE_HEADERS_COMPOSE,
	C2_SHOWABLE_HEADERS_SAVE,
	C2_SHOWABLE_HEADERS_PRINT,
	C2_SHOWABLE_HEADERS_LAST
};

typedef enum
{
	C2_REPORT_MESSAGE,
	C2_REPORT_WARNING,
	C2_REPORT_ERROR,
	C2_REPORT_CRITICAL	/* Will finish the application */
} C2ReportSeverity;

typedef struct
{
	gchar *font;
	gboolean use_my_fonts;
	
	GdkColor bg_color;
	GdkColor fg_color;
} C2HTMLOptions;

struct C2Application
{
	GtkTooltips *tooltips;
	GList *open_windows;

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
c2_app_init										(void);

void
c2_app_register_window							(GtkWindow *window);

void
c2_app_unregister_window						(GtkWindow *window);

void
c2_app_report									(const gchar *msg, C2ReportSeverity severity);

void
c2_app_start_activity							(GtkWidget *progress);

void
c2_app_stop_activity							(void);

void
c2_install_new									(void);

void
c2_mailbox_tree_fill							(C2Mailbox *head, GtkCTreeNode *node,
												 GtkWidget *ctree, GtkWidget *window);

/* defined in preferences.c */
void
c2_preferences_new								(void);

#ifdef __cplusplus
}
#endif

#endif
