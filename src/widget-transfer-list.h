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
 *  GNU General Public License for more details._
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __WIDGET_TRANSFER_LIST_H__
#define __WIDGET_TRANSFER_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
		
#ifdef BUILDING_C2
#	include "widget-transfer-item.h"
#	include "widget-dialog.h"
#else
#	include <cronosII.h>
#endif

/* I am a little drunk while coding this, understand if I'm coding non sense */
#define C2_TYPE_TRANSFER_LIST				(c2_transfer_list_get_type ())
#define C2_TRANSFER_LIST(obj)				(GTK_CHECK_CAST (obj, C2_TYPE_TRANSFER_LIST, C2TransferList))
#define C2_TRANSFER_LIST_CLASS(klass)		(GTK_CHECK_CAST (klass, C2_TYPE_TRANSFER_LIST, C2TransferListClass))

typedef struct _C2TransferList C2TransferList;
typedef struct _C2TransferListClass C2TransferListClass;

struct _C2TransferList
{
	C2Dialog dialog;

	GtkTable *receive_table;
	GtkTable *send_table;

	GSList *receive_list;
	GSList *send_list;
};

struct _C2TransferListClass
{
	C2DialogClass parent_class;

	void (*finish) (C2TransferList *tl);
	void (*cancel_all) (C2TransferList *tl);
};

GtkType
c2_transfer_list_get_type					(void);

GtkWidget *
c2_transfer_list_new						(C2Application *application);

void
c2_transfer_list_add_item					(C2TransferList *tl, C2TransferItem *item);

#ifdef __cplusplus
}
#endif

#endif
