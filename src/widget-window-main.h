/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
#ifndef __WIDGET_WINDOW_MAIN_H__
#define __WIDGET_WINDOW_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <pthread.h>
#include <glade/glade.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/mailbox.h>
#	include <libcronosII/utils-mutex.h>
#	include "widget-application.h"
#	include "widget-window.h"
#else
#	include <cronosII.h>
#endif

#define C2_WINDOW_MAIN(obj)					(GTK_CHECK_CAST (obj, c2_window_main_get_type (), C2WindowMain))
#define C2_WINDOW_MAIN_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_window_main_get_type (), C2WindowMainClass))
#define C2_IS_WINDOW_MAIN(obj)				(GTK_CHECK_TYPE (obj, c2_window_main_get_type ()))
#define C2_WINDOW_MAIN_CLASS_FW(obj)		(C2_WINDOW_MAIN_CLASS (((GtkObject*)(obj))->klass))

typedef struct _C2WindowMain C2WindowMain;
typedef struct _C2WindowMainClass C2WindowMainClass;
typedef enum _C2WindowMainToolbarButton C2WindowMainToolbarButton;

enum _C2WindowMainToolbarButton
{
	C2_WINDOW_MAIN_TOOLBAR_CHECK,
	C2_WINDOW_MAIN_TOOLBAR_SEND,
	C2_WINDOW_MAIN_TOOLBAR_FIND,
	C2_WINDOW_MAIN_TOOLBAR_SAVE,
	C2_WINDOW_MAIN_TOOLBAR_PRINT,
	C2_WINDOW_MAIN_TOOLBAR_DELETE,
	C2_WINDOW_MAIN_TOOLBAR_COPY,
	C2_WINDOW_MAIN_TOOLBAR_MOVE,
	C2_WINDOW_MAIN_TOOLBAR_COMPOSE,
	C2_WINDOW_MAIN_TOOLBAR_REPLY,
	C2_WINDOW_MAIN_TOOLBAR_REPLY_ALL,
	C2_WINDOW_MAIN_TOOLBAR_FORWARD,
	C2_WINDOW_MAIN_TOOLBAR_PREVIOUS,
	C2_WINDOW_MAIN_TOOLBAR_NEXT,
	C2_WINDOW_MAIN_TOOLBAR_CONTACTS,
	C2_WINDOW_MAIN_TOOLBAR_CLOSE,
	C2_WINDOW_MAIN_TOOLBAR_EXIT,
	C2_WINDOW_MAIN_TOOLBAR_SPACE
};
	
struct _C2WindowMain
{
	C2Window window;
	
	GtkWidget *toolbar, *mlist, *index, *mail_vbox, *mail, *nt; /* Statically created widgets need a reference here */
	GladeXML *toolbar_menu;

	C2Mutex index_lock;
	C2Mutex body_lock;
};

struct _C2WindowMainClass
{
	C2WindowClass parent_class;

	void (*check) (C2WindowMain *wmain);
	void (*close) (C2WindowMain *wmain);
	void (*compose) (C2WindowMain *wmain);
	void (*contacts) (C2WindowMain *wmain);
	void (*copy) (C2WindowMain *wmain);
	void (*delete) (C2WindowMain *wmain);
	void (*exit) (C2WindowMain *wmain);
	void (*expunge) (C2WindowMain *wmain);
	void (*forward) (C2WindowMain *wmain);
	void (*move) (C2WindowMain *wmain);
	void (*next) (C2WindowMain *wmain);
	void (*previous) (C2WindowMain *wmain);
	void (*print) (C2WindowMain *wmain);
	void (*reply) (C2WindowMain *wmain);
	void (*reply_all) (C2WindowMain *wmain);
	void (*save) (C2WindowMain *wmain);
	void (*search) (C2WindowMain *wmain);
	void (*send) (C2WindowMain *wmain);
};

GtkType
c2_window_main_get_type						(void);

GtkWidget *
c2_window_main_new							(C2Application *application);

void
c2_window_main_construct					(C2WindowMain *wmain, C2Application *application);

void
c2_window_main_set_mailbox					(C2WindowMain *wmain, C2Mailbox *mailbox);

GtkObject *
c2_window_main_get_mlist_selection			(C2WindowMain *wmain);

/*********************
 * [Common Dialogs ] *
 *********************/
void
c2_window_main_add_mailbox_dialog			(C2WindowMain *wmain);

void
c2_window_main_edit_mailbox_dialog			(C2WindowMain *wmain);

void
c2_window_main_remove_mailbox_dialog		(C2WindowMain *wmain);

void
c2_window_main_toolbar_configuration_dialog	(C2WindowMain *wmain);

#ifdef __cplusplus
}
#endif

#endif
