/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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

#ifdef USE_DEBUG
#	define C2_APPLICATION(obj)				(GTK_CHECK_CAST (obj, c2_application_get_type (), C2Application))
#	define C2_APPLICATION_CLASS(klass)		(GTK_CHECK_CLASS_CAST (klass, c2_application_get_type (), C2ApplicationClass))
#else
#	define C2_APPLICATION(obj)				((C2Application*)obj)
#	define C2_APPLICATION_CLASS(klass)		((C2ApplicationClass*)klass)
#endif

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

#define C2_UNIX_SOCKET						"server"

#define C2_COMMAND_WINDOW_MAIN_NEW	"window main::new"
#define C2_COMMAND_WINDOW_MAIN_RAISE	"window main::raise"
#define C2_COMMAND_WINDOW_MAIN_HIDE	"window main::hide"
#define C2_COMMAND_COMPOSER_NEW		"composer::new"
#define C2_COMMAND_CHECK_MAIL		"check mail"
#define C2_COMMAND_EXIT				"exit"

typedef struct _C2Application C2Application;
typedef struct _C2ApplicationClass C2ApplicationClass;

#ifdef BUILDING_C2
#	include "widget-window.h"
#else
#	include <cronosII.h>
#endif

struct _C2Application
{
	GtkObject object;

	guint check_timeout;
	
	GSList *open_windows;
	GSList *tmp_files;

	GdkBitmap *mask_i_unread, *mask_i_read, *mask_i_reply, *mask_i_forward,
			  *mask_unread, *mask_read, *mask_reply, *mask_forward;
	GdkPixmap *pixmap_i_unread, *pixmap_i_read, *pixmap_i_reply, *pixmap_i_forward,
			  *pixmap_unread, *pixmap_read, *pixmap_reply, *pixmap_forward;
	GdkCursor *cursor_busy;
	GdkCursor *cursor_normal;

	gchar *name;

	gint server_socket;
	/* The difference between the following two booleans is that
	 * acting_as_server is %TRUE when this application is being
	 * the server (owner of ~/.c2/server), but that the once the
	 * last window is closed it will be shutdown.
	 * The running_as_server is %TRUE when the porpose of its
	 * existence is to be a server. When running_as_server is %TRUE
	 * the application won't exit until the remote command Exit
	 * is called.
	 */
	gint acting_as_server : 1;
	gint running_as_server : 1;
	C2Mutex *server_lock;

	C2Mailbox *mailbox;

	C2Account *account;

	GdkFont *fonts_gdk_readed_mails;
	GdkFont *fonts_gdk_unreaded_mails;
	GdkFont *fonts_gdk_readed_mailbox;
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

	/* Pre-packed functions */
	/* "open_message", "reply", "reply_all", "forward":
	 *      If you know the @db arg, use it and set @message to %NULL,
	 *      if you don't set the @db arg to %NULL and use the @message arg.
	 */
	void (*check) (C2Application *application);
	void (*copy) (C2Application *application, GList *list, C2Window *window);
	void (*delete) (C2Application *application, GList *list, C2Window *window);
	void (*expunge) (C2Application *application, GList *list, C2Window *window);
	void (*forward) (C2Application *application, C2Db *db, C2Message *message);
	void (*move) (C2Application *application, GList *list, C2Window *window);
	void (*open_message) (C2Application *application, C2Db *db, C2Message *message);
	void (*print) (C2Application *application, C2Message *message);
	void (*reply) (C2Application *application, C2Db *db, C2Message *message);
	void (*reply_all) (C2Application *application, C2Db *db, C2Message *message);
	void (*save) (C2Application *application, C2Message *message, C2Window *window);
	void (*send) (C2Application *application);
};

GtkType
c2_application_get_type						(void);

C2Application *
c2_application_new							(const gchar *name);

void
c2_application_running_as_server			(C2Application *application);

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

/****************
 * [ Commands ] *
 ****************/

void
c2_application_command						(C2Application *application, const gchar *cmnd, gpointer edata);

/***********************
 * [Application Utils] *
 ***********************/
gboolean
c2_application_check_account_exists			(C2Application *application);

gint
c2_application_get_mailbox_configuration_id_by_name	(const gchar *name);

/* install.c */
void
c2_install_new								(void);

void
c2_mailbox_tree_fill				(C2Mailbox *head, GtkCTreeNode *node, GtkWidget *ctree, GtkWidget *window);

/* preferences.c */
void
c2_preferences_new				(void);

#ifdef __cplusplus
}
#endif

#endif
