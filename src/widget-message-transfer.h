/*  Cronos II Mail Client /src/widget-message-transfer.h
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
#ifndef __WIDGET_MESSAGE_TRANSFER_H__
#define __WIDGET_MESSAGE_TRANSFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <glade/glade.h>
#include <libgnomeui/gnome-dialog.h>
#include <pthread.h>

#ifdef BUILDING_C2
#	include <libcronosII/account.h>
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_MESSAGE_TRANSFER				(c2_message_transfer_get_type ())
#define C2_MESSAGE_TRANSFER(obj)			(GTK_CHECK_CAST (obj, C2_TYPE_MESSAGE_TRANSFER, C2MessageTransfer))
#define C2_MESSAGE_TRANSFER_CLASS(klass)		(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_MESSAGE_TRANSFER, C2MessageTransferClass))
#define C2_IS_MESSAGE_TRANSFER(obj)				(GTK_CHECK_TYPE (obj, C2_TYPE_MESSAGE_TRANSFER))

typedef struct _C2MessageTransfer C2MessageTransfer;
typedef struct _C2MessageTransferClass C2MessageTransferClass;
typedef struct _C2MessageTransferQueue C2MessageTransferQueue;
typedef enum _C2MessageTransferAction C2MessageTransferAction;
typedef enum _C2MessageTransferType C2MessageTransferType;

enum _C2MessageTransferAction
{
	C2_MESSAGE_TRANSFER_CHECK,
	C2_MESSAGE_TRANSFER_SEND
};

enum _C2MessageTransferType
{
	C2_MESSAGE_TRANSFER_AUTOMATIC,
	C2_MESSAGE_TRANSFER_MANUAL
};

enum
{
	DONE,
	TOTAL,
	LAST_CATEGORY
};

struct _C2MessageTransferQueue
{
	C2MessageTransferAction action;
	C2MessageTransferType type;
	const C2Account *account;
	gpointer edata;

	C2MessageTransferQueue *next;
};

struct _C2MessageTransfer
{
	GnomeDialog dialog;

	guint16 tasks[LAST_CATEGORY];
	guint32 subtasks[LAST_CATEGORY];
	guint32 subtask[LAST_CATEGORY];

	C2MessageTransferQueue *queue;
	pthread_mutex_t queue_lock;

	GladeXML *xml;

	gint close	: 1; /* Automatically close when finished. */
	gint cancel	: 1; /* Button 'Cancel' has been pressed. */
	gint active	: 1; /* The queue is being processed? */
};

struct _C2MessageTransferClass
{
	GnomeDialogClass parent_class;

	void (*append) (C2MessageTransfer *mt, C2MessageTransferAction action, C2Account *account);
	void (*task_done) (C2MessageTransfer *mt, gint number);
};

GtkType
c2_message_transfer_get_type					(void);

GtkWidget *
c2_message_transfer_new							(void);

void
c2_message_transfer_freeze						(C2MessageTransfer *mt);

void
c2_message_transfer_thaw						(C2MessageTransfer *mt);

void
c2_message_transfer_append						(C2MessageTransfer *mt, const C2Account *account,
												 C2MessageTransferType type,
												 C2MessageTransferAction action, ...);

void
c2_message_transfer_get_info					(C2MessageTransfer *mt, gint i, C2Account **account,
												 C2MessageTransferAction *action, gint messages);

#ifdef __cplusplus
}
#endif

#endif
