/*  Cronos II Mail Client /src/widget-message-transfer.c
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
#include <gnome.h>
#include <glade/glade.h>

#include <libcronosII/error.h>

#include "c2-app.h"
#include "widget-message-transfer.h"

static void
class_init										(C2MessageTransferClass *klass);

static void
init											(C2MessageTransfer *obj);

static void
destroy											(GtkObject *obj);

static C2MessageTransferQueue *
queue_new										(const C2Account *account, C2MessageTransferType type, 
												 C2MessageTransferAction action, ...);

static C2MessageTransferQueue *
queue_append									(C2MessageTransferQueue *head, C2MessageTransferQueue *obj);

static void
queue_free										(C2MessageTransferQueue *head);

static void
run												(C2MessageTransfer *mt);

static void
reset											(C2MessageTransfer *mt);

enum
{
	APPEND,
	TASK_DONE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GnomeDialogClass *parent_class = NULL;

GtkWidget *
c2_message_transfer_new (void)
{
	C2MessageTransfer *mt = gtk_type_new (c2_message_transfer_get_type ());
	GtkWidget *box;

	mt->xml = glade_xml_new (C2_APP_GLADE_FILE ("cronosII"), "dlg_message_transfer_box");
	box = glade_xml_get_widget (mt->xml, "dlg_message_transfer_box");

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (mt)->vbox), box, TRUE, TRUE, 0);

	return GTK_WIDGET (mt);
}

void
c2_message_transfer_freeze (C2MessageTransfer *mt)
{
	/* XXX If we get some problems, like gtk+ freezing when
	 * XXX some checking stuff is executed, there's a good
	 * XXX chance the problem is here (we are using a blocking
	 * XXX function.
	 */
	pthread_mutex_lock (&mt->queue_lock);
}

void
c2_message_transfer_thaw (C2MessageTransfer *mt)
{
	pthread_mutex_unlock (&mt->queue_lock);

	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "task_progress")),
							mt->tasks[DONE], 0, mt->tasks[TOTAL]);
	
	/* If it is not already running, make it run */
	if (!mt->active)
		run (mt);
}

void
c2_message_transfer_append (C2MessageTransfer *mt, const C2Account *account, C2MessageTransferType type,
							C2MessageTransferAction action, ...)
{
	GtkWidget *tasks = glade_xml_get_widget (mt->xml, "task_progress");
	C2MessageTransferQueue *queue;
	va_list args;
	
	queue = queue_new (account, type, action);
	mt->queue = queue_append (mt->queue, queue);

	mt->tasks[TOTAL]++;
}

static C2MessageTransferQueue *
queue_new (const C2Account *account, C2MessageTransferType type, C2MessageTransferAction action, ...)
{
	C2MessageTransferQueue *queue = g_new0 (C2MessageTransferQueue, 1);

	queue->action = action;
	queue->type = type;
	queue->account = account;
	queue->edata = NULL;

	/* TODO read the ... */

	return queue;
}

static C2MessageTransferQueue *
queue_append (C2MessageTransferQueue *head, C2MessageTransferQueue *obj)
{
	C2MessageTransferQueue *s;
	
	c2_return_val_if_fail (obj, head, C2EDATA);

	for (s = head; s; s = s->next)
		if (!s->next)
			break;

	if (s)
		s->next = obj;
	else
		head = obj;

	return head;
}

static void
queue_free (C2MessageTransferQueue *head)
{
}

static void
run (C2MessageTransfer *mt)
{
	C2MessageTransferQueue *s;
	gchar *info = NULL;
	gboolean success = TRUE;

	/* We need to block here because we don't want
	 * two run running at the same time.
	 */
	c2_return_if_fail (!mt->active, C2EDATA);

	mt->active = TRUE;
	reset (mt);

	for (s = mt->queue; s; s = s->next)
	{
		if (s->action == C2_MESSAGE_TRANSFER_CHECK)
		{
			/* Check an account */
			g_print ("The checking of accounts is not coded yet: %s:%d\n", __FILE__, __LINE__);
		} else
		{
			/* Send a mail */
			g_print ("The sending of messages is not coded yet: %s:%d\n", __FILE__, __LINE__);
		}
	}

	if (pthread_mutex_trylock (&mt->queue_lock))
	{
		mt->active = FALSE;
		return;
	}

	queue_free (mt->queue);
	mt->active = FALSE;
	reset (mt);

	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (mt->xml, "auto_close_btn"))->active)
		gtk_widget_hide (GTK_WIDGET (mt));
	
	pthread_mutex_unlock (&mt->queue_lock);
}

static void
reset (C2MessageTransfer *mt)
{
	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "task_progress")), 0, 0, 0);
	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "subtasks_progress")), 0, 0, 0);
	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "subtask_progress")), 0, 0, 0);
	gtk_widget_hide (glade_xml_get_widget (mt->xml, "subtask_progress"));
	gtk_clist_clear (GTK_CLIST (glade_xml_get_widget (mt->xml, "task_clist")));
}

GtkType
c2_message_transfer_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2MessageTransfer",
			sizeof (C2MessageTransfer),
			sizeof (C2MessageTransferClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gnome_dialog_get_type (), &info);
	}

	return type;
}

static void
class_init (C2MessageTransferClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gnome_dialog_get_type ());

	signals[APPEND] =
		gtk_signal_new ("append",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (C2MessageTransferClass, append),
				gtk_marshal_NONE__ENUM, GTK_TYPE_NONE, 2,
				GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	signals[TASK_DONE] =
		gtk_signal_new ("task_done",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (C2MessageTransferClass, task_done),
				gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
				GTK_TYPE_INT);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->append = NULL;
	klass->task_done = NULL;

	object_class->destroy = destroy;
}

static void
init (C2MessageTransfer *obj)
{
	gchar *buttons[] =
	{
		GNOME_STOCK_BUTTON_OK,
		GNOME_STOCK_BUTTON_CANCEL,
		NULL
	};
	
	obj->tasks[DONE] = obj->tasks[TOTAL] =
	obj->subtasks[DONE] = obj->subtasks[TOTAL] =
	obj->subtask[DONE], obj->subtask[TOTAL] = 0;

	obj->queue = NULL;
	obj->xml = NULL;
	obj->close = FALSE;
	obj->cancel = FALSE;

	pthread_mutex_init (&obj->queue_lock, NULL);

	gnome_dialog_constructv (GNOME_DIALOG (obj), _("Message Transfer"), (const gchar **) buttons);
}

static void
destroy (GtkObject *obj)
{
	C2MessageTransfer *mt = C2_MESSAGE_TRANSFER (obj);

	queue_free (mt->queue);

	gtk_object_destroy (GTK_OBJECT (mt->xml));
}
