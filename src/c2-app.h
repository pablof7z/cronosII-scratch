/*  Cronos II Mail Client
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
#ifndef __CRONOSII_APP_H__
#define __CRONOSII_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#	include <libcronosII/account.h>
#	include <libcronosII/mailbox.h>
#	include <libcronosII/utils.h>
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

typedef enum
{
	C2_DEFAULT_MIME_PLAIN,
	C2_DEFAULT_MIME_HTML
} C2DefaultMimeType;

#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
#	define DEFAULT_PART							C2_DEFAULT_MIME_HTML
#else
#	define DEFAULT_PART							C2_DEFAULT_MIME_PLAIN
#endif

typedef enum
{
	C2_FONT_USE_DOCUMENTS_FONT,
	C2_FONT_USE_MY_FONT
} C2FontSource;

typedef enum
{
	C2_COLOR_USE_DOCUMENTS_COLORS,
	C2_COLOR_USE_MY_COLOR
} C2ColorSource;

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
	GList *tmp_files;

	GdkBitmap *mask_unread, *mask_read, *mask_reply, *mask_forward;
	GdkPixmap *pixmap_unread, *pixmap_read, *pixmap_reply, *pixmap_forward;
	GdkCursor *cursor_busy;
	GdkCursor *cursor_normal;
	GdkFont *gdk_font_body;
	GdkFont *gdk_font_read;
	GdkFont *gdk_font_unread;

	C2Mailbox *mailbox;

	gint options_check_timeout;
	gint options_mark_timeout;
	gchar *options_prepend_character;
	gint options_empty_garbage	: 1;
	gint options_use_outbox		: 1;
	gint options_check_at_start	: 1;
	C2DefaultMimeType options_default_mime;
		
	C2Account *account;

	gchar *interface_title;
	GtkToolbarStyle interface_toolbar;
	gchar *interface_date_fmt;

	gchar *fonts_message_body;
	gchar *fonts_unreaded_message;
	gchar *fonts_readed_message;
	C2FontSource fonts_source;

	GdkColor colors_replying_original_message;
	GdkColor colors_message_bg;
	GdkColor colors_message_fg;
	C2ColorSource colors_message_source;

	gchar *paths_saving_entry;
	gchar *paths_download_entry;
	gchar *paths_get_entry;

	gchar *advanced_http_proxy_addr;
	gint advanced_http_proxy_port;
	gint advanced_http_proxy : 1;
	gchar *advanced_ftp_proxy_addr;
	gint advanced_ftp_proxy_port;
	gint advanced_ftp_proxy : 1;
	gchar *advanced_persistent_smtp_addr;
	gint advanced_persistent_smtp_port;
	gint advanced_persistent_smtp : 1;
	gint advanced_use_internal_browser : 1;
	
	gint wm_hpan;
	gint wm_vpan;
	gint wm_clist[8];
	gint wm_width;
	gint wm_height;
	
	guint showable_headers[C2_SHOWABLE_HEADERS_LAST];
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
