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

#define C2_APPLICATION_GLADE_FILE(x)		(GLADEDIR G_DIR_SEPARATOR_S x ".glade")

#define C2_APPLICATION(obj)					(GTK_CHECK_CAST (obj, c2_application_get_type (), C2Application))
#define C2_APPLICATION_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_application_get_type (), C2ApplicationClass))
#define C2_IS_APPLICATION(obj)				(GTK_CHECK_TYPE (obj, c2_application_get_type ()))
#define C2_APPLICATION_CLASS_FW(obj)		(C2_APPLICATION_CLASS (((GtkObject*)(obj))->klass))

#define C2_MAILBOX_INBOX					_("Inbox")
#define C2_MAILBOX_OUTBOX					_("Outbox")
#define C2_MAILBOX_SENT_ITEMS				_("Sent Items")
#define C2_MAILBOX_TRASH					_("Trash")
#define C2_MAILBOX_DRAFTS					_("Drafts")
#define C2_MAILBOX_N_INBOX					N_("Inbox")
#define C2_MAILBOX_N_OUTBOX					N_("Outbox")
#define C2_MAILBOX_N_SENT_ITEMS				N_("Sent Items")
#define C2_MAILBOX_N_TRASH					N_("Trash")
#define C2_MAILBOX_N_DRAFTS					N_("Drafts")

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

typedef struct _C2Application C2Application;
typedef struct _C2ApplicationClass C2ApplicationClass;
typedef struct _C2ApplicationReportType C2ApplicationReportType;

#ifdef BUILDING_C2
#	include "widget-window.h"
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

	C2Account *account;

	GdkFont *fonts_gdk_readed_mails;
	GdkFont *fonts_gdk_unreaded_mails;
	GdkFont *fonts_gdk_unreaded_mailbox;
	GdkFont *fonts_gdk_composer_body;
	GdkFont *fonts_gdk_message_body;
};

struct _C2ApplicationClass
{
	GtkObjectClass parent_class;

	void (*application_preferences_changed) (C2Application *application, gint key,
												gpointer value);
	void (*reload_mailboxes) (C2Application *application);
	void (*window_changed) (C2Application *application, GSList *list);

	void (*send) (C2Application *application);
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

GtkWidget *
c2_application_window_get					(C2Application *application, const gchar *type);

GSList *
c2_application_open_windows					(C2Application *application);

/***********************
 * [Application Utils] *
 ***********************/
gboolean
c2_application_check_account_exists			(C2Application *application);

/* install.c */
void
c2_install_new								(void);

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
