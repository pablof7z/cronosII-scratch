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
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "message.h"
#include "error.h"
#include "utils.h"

enum
{
	STATUS_CHANGE,
	MESSAGE_ACTION,
	LAST_SIGNAL
};

static GtkObjectClass *parent_class = NULL;
static guint c2_message_signals[LAST_SIGNAL] = { 0 };

gchar *
c2_message_get_header_field (C2Message *message, const gchar *field)
{
	const gchar *msg_ptr;
	const gchar *start_ptr, *end_ptr;
	gchar *chunk = NULL, *ptr;
	size_t size, wbytes;
	
	c2_return_val_if_fail (field, NULL, C2EDATA);

	if (!message->header)
		if (!c2_message_get_message_header (message))
			return NULL;

	/* Search for the field */
	if (!(msg_ptr = c2_strstr_case_insensitive (msg_ptr, field)))
		return NULL;

	/* Set a pointer to the start of the value */
	start_ptr = msg_ptr+strlen (field);
	if (*start_ptr == ':')
		ptr++;
	for (; *start_ptr != '\0' && *start_ptr == ' '; start_ptr++);

	/* Calculate the size of the chunk */
	for (end_ptr = start_ptr, size = 0; *end_ptr != '\0'; end_ptr++)
	{
		if (*end_ptr != '\n')
			size++;
		else
		{
			if (*(++end_ptr) == '\t' || *end_ptr == ' ')
			{
				size++;
				/* Go through the '\t''s and the white spaces */
				/* Duplicated on porpouse: So if there's a "\t \t " it will successfully
				 * parse */
				while (*end_ptr == '\t') end_ptr++;
				while (*end_ptr == ' ') end_ptr++;
				while (*end_ptr == '\t') end_ptr++;
				while (*end_ptr == ' ') end_ptr++;
				end_ptr--;
			}
			else
				break;
		}
	}

	/* Alloc the chunk */
	chunk = g_new0 (gchar, size+1);

	for (ptr = chunk, wbytes = 0; wbytes < size+1 && *start_ptr != '\0'; start_ptr++)
	{
		if (*start_ptr != '\n')
		{
			*(ptr++) = *start_ptr;
			wbytes++;
		}
		else
		{
			if (*(++start_ptr) == '\t' || *start_ptr == ' ')
			{
				*(ptr++) = ' ';
				wbytes++;
				while (*start_ptr == '\t') start_ptr++;
				while (*start_ptr == ' ') start_ptr++;
				while (*start_ptr == '\t') start_ptr++;
				while (*start_ptr == ' ') start_ptr++;
				start_ptr--;
			}
			else
				break;
		}
	}
	chunk[wbytes] = 0;

	return chunk;
}

/**
 * c2_message_get_message_header
 * @message: Message in C2Message object.
 *
 * Gets a header from the message source.
 *
 * Return Value:
 * A newly allocated mem chunk with the header in it.
 **/
gchar *
c2_message_get_message_header (C2Message *message)
{
	const gchar *ptr;
	const gchar *msg_ptr;
	gchar *chunk;
	
	c2_return_val_if_fail (message->message, NULL, C2EDATA);

	/* Message header is recorder in C2Message objects,
	 * if it has already been asked for it, just return it.
	 */
	if (message && message->header)
		return message->header;
	
	if (!(ptr = strstr (msg_ptr, "\n\n")))
		return NULL;

	chunk = g_strndup (msg_ptr, ptr - msg_ptr);

	if (message)
		message->header = chunk;

	return chunk;
}

/**
 * c2_message_get_message_body
 * @message: Message in C2Message object.
 *
 * Locates the start of the message body;
 *
 * Return Value:
 * A pointer to the start of the body within the argument.
 **/
const gchar *
c2_message_get_message_body (C2Message *message)
{
	const gchar *msg_ptr;
	const gchar *ptr;
	
	c2_return_val_if_fail (message->message, NULL, C2EDATA);

	if (message->body)
		return message->body;
	
	if (!(ptr = strstr (msg_ptr, "\n\n")))
		return NULL;

	if (message)
		message->body = ptr;

	return ptr;
}

void
c2_message_destroy (C2Message *message)
{
	c2_return_if_fail (message, C2EDATA);

	g_free (message->mbox);
	g_free (message->message);
	g_free (message->header);
	g_list_free (message->mime);
	g_free (message);
}

static void
c2_message_init (C2Message *message)
{
	message->message = NULL;
	message->header = NULL;
	message->body = NULL;
	message->mime = NULL;
	message->node = NULL;
}

static void
c2_message_class_init (C2MessageClass *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gtk_object_get_type ());
	
	c2_message_signals[STATUS_CHANGE] =
	gtk_signal_new ("status-change",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (C2MessageClass, status_change),
			gtk_marshal_NONE__ENUM, GTK_TYPE_NONE, 1,
			GTK_TYPE_ENUM);
	
	c2_message_signals[MESSAGE_ACTION] =
	gtk_signal_new ("message-action",
			GTK_RUN_FIRST,
			object_class->type,
			GTK_SIGNAL_OFFSET (C2MessageClass, message_action),
			gtk_marshal_NONE__ENUM, GTK_TYPE_NONE, 2,
			GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	
	gtk_object_class_add_signals (object_class, c2_message_signals, LAST_SIGNAL);
	
	object_class->destroy = c2_message_destroy;
	
	klass->status_change = c2_message_status_change;
	klass->message_action = c2_message_message_action;
}

C2Message *
c2_message_new (void)
{
	return gtk_type_new (C2_TYPE_MESSAGE);
}

GtkType
c2_message_get_type (void)
{
	static GtkType c2_message_type = 0;
	
	if (!c2_message_type) {
		static const GtkTypeInfo c2_message_info = {
			"C2Message",
			sizeof (C2Message),
			sizeof (C2MessageClass),
			(GtkClassInitFunc) c2_message_class_init,
			(GtkObjectInitFunc) c2_message_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		
		c2_message_type =
			gtk_type_unique (gtk_object_get_type (), &c2_message_info);
	}
	
	return c2_message_type;
}
