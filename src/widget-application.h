/*  Cronos II - The GNOME mail client
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
#ifndef __WIDGET_APPLICATION_H__
#define __WIDGET_APPLICATION_H__

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

#define C2_APPLICATION_GLADE_FILE(x)		(PKGDATADIR G_DIR_SEPARATOR_S x ".glade")

#define C2_APPLICATION(obj)					(GTK_CHECK_CAST (obj, c2_application_get_type (), C2Application))
#define C2_APPLICATION_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_application_get_type (), C2ApplicationClass))

#define C2_MAILBOX_INBOX					_("Inbox")
#define C2_MAILBOX_OUTBOX					_("Outbox")
#define C2_MAILBOX_QUEUE					_("Queue")
#define C2_MAILBOX_GARBAGE					_("Garbage")
#define C2_MAILBOX_DRAFTS					_("Drafts")

#define DEFAULT_OPTIONS_CHECK_TIMEOUT		"20"
#define DEFAULT_OPTIONS_WRAP_OUTGOING_TEXT	"70"
#define DEFAULT_OPTIONS_MARK_TIMEOUT		"1"
#define DEFAULT_OPTIONS_PREPEND_CHARACTER	"> "
#define DEFAULT_OPTIONS_EMPTY_GARBAGE		"0"
#define DEFAULT_OPTIONS_USE_OUTBOX			"1"
#define DEFAULT_OPTIONS_CHECK_AT_START		"0"
#define DEFAULT_OPTIONS_MT_MODE				"1"
#if defined (USE_GTKHTML) || defined (USE_GTKXMHTML)
#	define DEFAULT_OPTIONS_DEFAULT_MIME		"1"
#else
#	define DEFAULT_OPTIONS_DEFAULT_MIME		"0"
#endif

#define DEFAULT_INTERFACE_TITLE				"Cronos II v.%v - %M: %m messages, %n new"
#define DEFAULT_INTERFACE_TOOLBAR			"2"
#define DEFAULT_INTERFACE_DATE_FMT			"%d.%m.%Y %H:%M"
	
#define DEFAULT_FONTS_MESSAGE_BODY			"-adobe-courier-medium-r-normal-*-*-120-*-*-m-*-iso8859-1"
#define DEFAULT_FONTS_UNREADED_MESSAGE		"-b&h-lucida-bold-r-normal-*-*-100-*-*-p-*-iso8859-1"
#define DEFAULT_FONTS_READED_MESSAGE		"-b&h-lucida-medium-r-normal-*-*-100-*-*-p-*-iso8859-1"
#define DEFAULT_FONTS_UNREADED_MAILBOX		"-adobe-helvetica-bold-r-normal-*-12-*-*-*-p-*-iso8859-1"
#define DEFAULT_FONTS_SOURCE				"0"

#define DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_RED	"0"
#define DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_GREEN	"0"
#define DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_BLUE	"65535"
#define DEFAULT_COLORS_MESSAGE_BG_RED		"65535"
#define DEFAULT_COLORS_MESSAGE_BG_GREEN		"65535"
#define DEFAULT_COLORS_MESSAGE_BG_BLUE		"65535"
#define DEFAULT_COLORS_MESSAGE_FG_RED		"0"
#define DEFAULT_COLORS_MESSAGE_FG_GREEN		"0"
#define DEFAULT_COLORS_MESSAGE_FG_BLUE		"0"
#define DEFAULT_COLORS_MESSAGE_SOURCE		"0"

#define DEFAULT_PATHS_SAVING				g_get_home_dir ()
#define DEFAULT_PATHS_DOWNLOAD				g_get_home_dir ()
#define DEFAULT_PATHS_GET					g_get_home_dir ()
#define DEFAULT_PATHS_ALWAYS_USE			"1"

#define DEFAULT_ADVANCED_HTTP_PROXY_ADDR	""
#define DEFAULT_ADVANCED_HTTP_PROXY_PORT	"80"
#define DEFAULT_ADVANCED_HTTP_PROXY			"0"
#define DEFAULT_ADVANCED_FTP_PROXY_ADDR		""
#define DEFAULT_ADVANCED_FTP_PROXY_PORT		"80"
#define DEFAULT_ADVANCED_FTP_PROXY			"0"
#define DEFAULT_ADVANCED_PERSISTENT_SMTP_ADDR	"localhost"
#define DEFAULT_ADVANCED_PERSISTENT_SMTP_PORT	"25"
#define DEFAULT_ADVANCED_PERSISTENT_SMTP	"0"
#define DEFAULT_ADVANCED_USE_INTERNAL_BROWSER	"1"
#define DEFAULT_ADVANCED_LOAD_MAILBOXES_AT_START "1"

#define DEFAULT_RC_HPAN						"150"
#define DEFAULT_RC_VPAN						"170"
#define DEFAULT_RC_CLIST_0					"22"
#define DEFAULT_RC_CLIST_1					"0"
#define DEFAULT_RC_CLIST_2					"0"
#define DEFAULT_RC_CLIST_3					"300"
#define DEFAULT_RC_CLIST_4					"280"
#define DEFAULT_RC_CLIST_5					"110"
#define DEFAULT_RC_CLIST_6					"10"
#define DEFAULT_RC_CLIST_7					"10"
#define DEFAULT_RC_WIDTH					"1024" /* XXX */
#define DEFAULT_RC_HEIGHT					"768" /* XXX */
#define DEFAULT_RC_SHOWABLE_HEADERS_PREVIEW	"9"
#define DEFAULT_RC_SHOWABLE_HEADERS_MESSAGE	"63"
#define DEFAULT_RC_SHOWABLE_HEADERS_COMPOSE	"61"
#define DEFAULT_RC_SHOWABLE_HEADERS_SAVE	"61"
#define DEFAULT_RC_SHOWABLE_HEADERS_PRINT	"61"

typedef struct _C2Application C2Application;
typedef struct _C2ApplicationClass C2ApplicationClass;
typedef struct _C2ApplicationReportType C2ApplicationReportType;

#ifdef BUILDING_C2
#	include "widget-window.h"
//#	include "widget-window-preferences.h"
#else
#	include <cronosII.h>
#endif

typedef enum
{
	C2_DEFAULT_MIME_PLAIN,
	C2_DEFAULT_MIME_HTML
} C2DefaultMimeType;

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
	C2_SHOWABLE_HEADER_FIELD_TO		= 1 << 1,
	C2_SHOWABLE_HEADER_FIELD_DATE		= 1 << 2,
	C2_SHOWABLE_HEADER_FIELD_FROM		= 1 << 3,
	C2_SHOWABLE_HEADER_FIELD_SUBJECT	= 1 << 4,
	C2_SHOWABLE_HEADER_FIELD_ACCOUNT	= 1 << 5,
	C2_SHOWABLE_HEADER_FIELD_CC		= 1 << 6,
	C2_SHOWABLE_HEADER_FIELD_BCC		= 1 << 7,
	C2_SHOWABLE_HEADER_FIELD_PRIORITY	= 1 << 8
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

enum _C2ApplicationReportType
{
	C2_APPLICATION_REPORT_MESSAGE,
	C2_APPLICATION_REPORT_WARNING,
	C2_APPLICATION_REPORT_ERROR,
	C2_APPLICATION_REPORT_CRITICAL	/* Will finish the application */
};

typedef struct
{
	gchar *font;
	gboolean use_my_fonts;
	
	GdkColor bg_color;
	GdkColor fg_color;
} C2HTMLOptions;

struct _C2Application
{
	GtkObject object;

	guint check_timeout;
	
	GSList *open_windows;
	GSList *tmp_files;

	GdkBitmap *mask_unread, *mask_read, *mask_reply, *mask_forward;
	GdkPixmap *pixmap_unread, *pixmap_read, *pixmap_reply, *pixmap_forward;
	GdkCursor *cursor_busy;
	GdkCursor *cursor_normal;

	gchar *name;

	C2Mailbox *mailbox;

	gint options_check_timeout;
	gint options_wrap_outgoing_text;
	gint options_mark_timeout;
	gchar *options_prepend_character;
	gint options_empty_garbage	: 1;
	gint options_use_outbox		: 1;
	gint options_check_at_start	: 1;
	gint options_mt_mode;
	C2DefaultMimeType options_default_mime;
		
	C2Account *account;

	gchar *interface_title;
	GtkToolbarStyle interface_toolbar;
	gchar *interface_date_fmt;

	gchar *fonts_message_body;
	gchar *fonts_unreaded_message;
	gchar *fonts_readed_message;
	gchar *fonts_unreaded_mailbox;
	GdkFont *fonts_gdk_message_body;
	GdkFont *fonts_gdk_unreaded_message;
	GdkFont *fonts_gdk_readed_message;
	GdkFont *fonts_gdk_unreaded_mailbox;
	C2FontSource fonts_source;

	GdkColor colors_replying_original_message;
	GdkColor colors_message_bg;
	GdkColor colors_message_fg;
	C2ColorSource colors_message_source;

	gchar *paths_saving;
	gchar *paths_download;
	gchar *paths_get;
	gint paths_always_use : 1;

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
	gint advanced_load_mailboxes_at_start : 1;
	
	gint rc_hpan;
	gint rc_vpan;
	gint rc_clist[8];
	gint rc_width;
	gint rc_height;
	guint rc_showable_headers[C2_SHOWABLE_HEADERS_LAST];
};

struct _C2ApplicationClass
{
	GtkObjectClass parent_class;

	void (*application_preferences_changed) (C2Application *application, gint key,
												gpointer value);
	void (*reload_mailboxes) (C2Application *application);
	void (*window_changed) (C2Application *application);
};

GtkType
c2_application_get_type						(void);

C2Application *
c2_application_new							(const gchar *name);

/********************
 * [Window Actions] *
 ********************/

void
c2_application_window_add					(C2Application *application, GtkWindow *window);

void
c2_application_window_remove				(C2Application *application, GtkWindow *window);

/***********************
 * [Application Utils] *
 ***********************/
gboolean
c2_application_check_account_exists			(C2Application *application);

void
c2_install_new					(void);

gint
c2_app_get_mailbox_configuration_id_by_name	(const gchar *name);

void
c2_mailbox_tree_fill				(C2Mailbox *head, GtkCTreeNode *node, GtkWidget *ctree, GtkWidget *window);

/* preferences.c */
void
c2_preferences_new				(void);

#ifdef __cplusplus
}
#endif

#endif
