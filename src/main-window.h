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
#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <pthread.h>

#ifdef HAVE_CONFIG_H
#	include <libmodules/mailbox.h>
#else
#	include <cronosII.h>
#endif
	
enum
{
	C2_HEADER_TITLES_FROM,
	C2_HEADER_TITLES_TO,
	C2_HEADER_TITLES_CC,
	C2_HEADER_TITLES_BCC,
	C2_HEADER_TITLES_SUBJECT,
	C2_HEADER_TITLES_ACCOUNT,
	C2_HEADER_TITLES_DATE,
	C2_HEADER_TITLES_PRIORITY,
	C2_HEADER_TITLES_LAST
};

typedef struct
{
	GtkWidget *get_new_mail;
	GtkWidget *sendqueue;
	GtkWidget *compose;
	GtkWidget *save;
	GtkWidget *print;
	GtkWidget *search;
	GtkWidget *_delete;
	GtkWidget *reply;
	GtkWidget *reply_all;
	GtkWidget *forward;
	GtkWidget *previous;
	GtkWidget *next;
#if DEBUG
	GtkWidget *debug;
#endif
	GtkWidget *quit;
} WindowMainToolbar;

typedef struct
{
	GtkWidget *file_menu;
	GtkWidget *get_new_mail;
	GtkWidget *get_new_mail_sep;
	GtkWidget *menu_sendqueue;
	GtkWidget *persistent_smtp_options;
	GtkWidget *persistent_smtp_options_connect;
	GtkWidget *persistent_smtp_options_disconnect;
	GtkWidget *quit;
	GtkWidget *edit_menu;
	GtkWidget *search;
	GtkWidget *message_menu;
	GtkWidget *compose;
	GtkWidget *save;
	GtkWidget *print;
	GtkWidget *reply;
	GtkWidget *reply_all;
	GtkWidget *forward;
	GtkWidget *copy;
	GtkWidget *move;
	GtkWidget *_delete;
	GtkWidget *expunge;
	GtkWidget *previous;
	GtkWidget *next;
	GtkWidget *mark;
	GtkWidget *settings;
	GtkWidget *preferences;
	GtkWidget *help;
	GtkWidget *about;
	GtkWidget *attach_menu;
	GtkWidget *attach_menu_sep;
} WindowMainMenubar;

typedef struct
{
	GtkWidget *window;
	GtkWidget *hpaned;
	GtkWidget *toolbar;
	GtkWidget *vpaned;
	GtkWidget *ctree;
	GtkWidget *index;
	pthread_mutex_t index_lock;
	GtkWidget *mime_left, *mime_right;
	GtkWidget *menu_clist;
	GtkWidget *header_table;
	GtkWidget *header_titles[C2_HEADER_TITLES_LAST][2];
	GtkWidget *text;
	pthread_mutex_t text_lock;
	GtkWidget *mime_scroll;
	GtkWidget *icon_list;
	GtkWidget *appbar;

	pthread_mutex_t appbar_lock;
	
	WindowMainToolbar tb_w;
	WindowMainMenubar mb_w;

	C2Mailbox *selected_mbox;
	gint selected_row;
} C2WindowMain;

C2WindowMain WMain;

void
c2_window_new									(void);

#ifdef __cplusplus
}
#endif

#endif
