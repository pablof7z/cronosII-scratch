/*  Cronos II - A GNOME mail client
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

static void
c2_message_class_init							(C2MessageClass *klass);

static void
c2_message_init									(C2Message *message);

static void
c2_message_destroy								(GtkObject *object);

enum
{
	MESSAGE_DIE,
	LAST_SIGNAL
};

static guint c2_message_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_message_get_type (void)
{
	static GtkType c2_message_type = 0;

	if (!c2_message_type)
	{
		static const GtkTypeInfo c2_message_info =
		{
			"C2Message",
			sizeof (C2Message),
			sizeof (C2MessageClass),
			(GtkClassInitFunc) c2_message_class_init,
			(GtkObjectInitFunc) c2_message_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_message_type = gtk_type_unique (gtk_object_get_type (), &c2_message_info);
	}

	return c2_message_type;
}

static void
c2_message_class_init (C2MessageClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	c2_message_signals[MESSAGE_DIE] =
		gtk_signal_new ("message_die",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2MessageClass, message_die),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	
	gtk_object_class_add_signals (object_class, c2_message_signals, LAST_SIGNAL);

	object_class->destroy = c2_message_destroy;

	klass->message_die = NULL;
}

static void
c2_message_init (C2Message *message)
{
	message->body = NULL;
	message->header = NULL;
	message->mime = NULL;
}

C2Message *
c2_message_new (void)
{
	return gtk_type_new (C2_TYPE_MESSAGE);
}

/**
 * c2_message_set_message
 * @message: The message that we need to work with.
 * @string: The message in standard RFC 821 format.
 *
 * This function will load the message @string
 * into the C2Message object @message.
 **/
void
c2_message_set_message (C2Message *message, const gchar *string)
{
	const gchar *ptr;
	
	c2_return_if_fail (message, C2EDATA);
	c2_return_if_fail (string, C2EDATA);

	if (!(ptr = strstr (string, "\n\n")))
		return;

	message->header = g_strndup (string, ptr-string);
	message->body = g_strdup (ptr+2);
}

static void
c2_message_destroy (GtkObject *object)
{
	C2Message *message;
	
	c2_return_if_fail (C2_IS_MESSAGE (object), C2EDATA);
	
	message = C2_MESSAGE (object);

	if (message->body)
		g_free (message->body);

	if (message->header)
		g_free (message->header);

	if (message->mime)
		gtk_object_unref (GTK_OBJECT (message->mime));
}

gchar *
c2_message_get_header_field (C2Message *message, const gchar *field)
{
	c2_return_val_if_fail (field, NULL, C2EDATA);

	if (!message->header)
		if (!c2_message_get_message_header (message))
			return NULL;

	return c2_message_str_get_header_field (message->header, field);
}

gchar *
c2_message_str_get_header_field (const gchar *message, const gchar *field)
{
	const gchar *msg_ptr;
	const gchar *start_ptr, *end_ptr;
	gchar *chunk = NULL, *ptr;
	size_t size, wbytes;
	
	/* Search for the field */
	if (!(msg_ptr = c2_strstr_case_insensitive (message, field)))
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
 * A non-freable mem chunk with the header in it.
 **/
const gchar *
c2_message_get_message_header (const C2Message *message)
{
	return message->header;
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
c2_message_get_message_body (const C2Message *message)
{
	return message->body;
}
