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
#ifndef __WIDGET_IMAP_LIST_H__
#define __WIDGET_IMAP_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <glade/glade.h>

#ifdef BUILDING_C2
#	include "widget-application.h"
#	include "widget-window.h"
#else
#	include <cronosII.h>
#endif

#define C2_IMAP_LIST(obj)					(GTK_CHECK_CAST (obj, c2_IMAP_list_get_type (), C2IMAPList))
#define C2_IMAP_LIST_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_IMAP_list_get_type (), C2IMAPListClass))
#define C2_IS_IMAP_LIST(obj)				(GTK_CHECK_TYPE (obj, c2_IMAP_list_get_type ()))

typedef struct _C2IMAPList C2IMAPList;
typedef struct _C2IMAPListClass C2IMAPListClass;

struct _C2IMAPList
{
	
};

struct _C2IMAPListClass
{
};

/* GTK+ general functions */
GtkType
c2_IMAP_list_get_type						(void);

GtkWidget *
c2_IMAP_list_new							(C2IMAP *imap);

#ifdef __cplusplus
}
#endif

#endif
