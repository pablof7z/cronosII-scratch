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
#ifndef __WIDGET_TRANSFER_ITEM_H__
#define __WIDGET_TRANSFER_ITEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
		
#ifdef BUILDING_C2
#	include "widget-dialog.h"
#	include <libcronosII/account.h>
#	include <libcronosII/message.h>
#	include <libcronosII/smtp.h>
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_TRANSFER_ITEM				(c2_transfer_item_get_type ())
#define C2_TRANSFER_ITEM(obj)				(GTK_CHECK_CAST (obj, C2_TYPE_TRANSFER_ITEM, C2TransferItem))
#define C2_TRANSFER_ITEM_CLASS(klass)		(GTK_CHECK_CAST (klass, C2_TYPE_TRANSFER_ITEM, C2TransferItemClass))
#define C2_IS_TRANSFER_ITEM(obj)			(GTK_CHECK_TYPE (obj, C2_TYPE_TRANSFER_ITEM))

typedef struct _C2TransferItem C2TransferItem;
typedef struct _C2TransferItemClass C2TransferItemClass;
typedef enum _C2TransferItemState C2TransferItemState;
typedef enum _C2TransferItemType C2TransferItemType;

enum _C2TransferItemState
{
	C2_TRANSFER_ITEM_CONNECT,
	C2_TRANSFER_ITEM_LOGIN,
	C2_TRANSFER_ITEM_TRANSFER,
	C2_TRANSFER_ITEM_DONE_ERROR,
	C2_TRANSFER_ITEM_DONE_SUCCESS
};

enum _C2TransferItemType
{
	C2_TRANSFER_ITEM_RECEIVE,
	C2_TRANSFER_ITEM_SEND
};

struct _C2TransferItem
{
	GtkObject object;

	C2TransferItemType type;
	C2TransferItemState state;
	gchar *tooltip;

	C2Account *account;
	C2Application *application;

	union
	{
		struct
		{
		} receive;
		struct
		{
			C2SMTP *smtp;
			C2Message *message;
		} send;
	} type_info;

	GtkWidget *table;

	GtkWidget *progress_mail;
	GtkWidget *progress_byte;
	GtkWidget *cancel_button;
};

struct _C2TransferItemClass
{
	GtkObjectClass parent_class;

	void (*state_changed) (C2TransferItem *ti, C2TransferItemState state);
	
	void (*finish) (C2TransferItem *ti);
	void (*cancel) (C2TransferItem *ti, C2Account *account);
};

GtkType
c2_transfer_item_get_type					(void);

C2TransferItem *
c2_transfer_item_new						(C2Application *application, C2Account *account,
											 C2TransferItemType type, ...);

void
c2_transfer_item_construct					(C2TransferItem *ti, C2Application *application,
											 C2Account *account, C2TransferItemType type, va_list args);

void
c2_transfer_item_start						(C2TransferItem *ti);

#ifdef __cplusplus
}
#endif

#endif
