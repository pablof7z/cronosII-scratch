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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "message.h"
#include "error.h"
#include "utils.h"

#define BOUNDARY_LENGTH 50

#define MIME_UNCAPABLE_WARNING "This is a multipart message in MIME format.\n" \
							   "Since your mail client is not MIME compatible\n" \
							   "you will not be able to see this message properly.\n" \
							   "You should strongly consider changing your\n" \
							   "mail client.\n" \
							   "You might want to send a mail to\n" \
							   "cronosII-hackers@lists.sourceforge.net for help\n" \
							   "or visit http://www.cronosII.net/\n"

static void
class_init									(C2MessageClass *klass);

static void
init										(C2Message *message);

static void
destroy										(GtkObject *object);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_message_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info =
		{
			"C2Message",
			sizeof (C2Message),
			sizeof (C2MessageClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2MessageClass *klass)
{
	GtkObjectClass *object_class;
	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
init (C2Message *message)
{
	message->body = NULL;
	message->header = NULL;
	message->mime = NULL;
}

C2Message *
c2_message_new (void)
{
	C2Message *message;

	message = gtk_type_new (C2_TYPE_MESSAGE);

	gtk_signal_connect (GTK_OBJECT (message), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

	return message;
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
	{
		/* This mail seems to be broken,
		 * lets try to load it anyway
		 * by seeing what looks like
		 * a header line and what not */
		const gchar *ptr2 = NULL;
		gboolean dots_reached;
		
		for (ptr = string; *ptr != '\0'; ptr++)
		{
			if (*ptr == ':')
					dots_reached = TRUE;
			if (*ptr == '\n')
			{
				if (dots_reached)
				{
					ptr2 = ptr+1;
					dots_reached = FALSE;
				} else
					break;
			}
		}

		if (!ptr2)
			return;
		ptr = ptr2;
	}

	message->header = g_strndup (string, ptr-string);
	for (; *ptr == '\n'; ptr++)
		;
	message->body = g_strdup (ptr);
}

static void
destroy (GtkObject *object)
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
	gchar *chunk = NULL, *ptr, *tmp;
	size_t size, wbytes, field_length;
	
	field_length = strlen (field);

	/* Search for the field */
	for (msg_ptr = message; *msg_ptr != '\0'; msg_ptr++)
		if ((msg_ptr == message) || (msg_ptr > message) && (*(msg_ptr-1) == '\n'))
		{
			if (c2_strneq (msg_ptr, field, field_length))
				break;
			else if (*msg_ptr == '\n')
			{
				msg_ptr = NULL;
				break;
			} else
			{
				for (; *msg_ptr != '\0' && *msg_ptr != '\n'; msg_ptr++)
					;

				if (*msg_ptr == '\0')
					msg_ptr--;
			}
		}
	
	if (!msg_ptr || *msg_ptr == '\0')
		return NULL;

	/* Set a pointer to the start of the value */
	start_ptr = msg_ptr+field_length;
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
				while (*end_ptr == '\t')
					end_ptr++;
				while (*end_ptr == ' ')
					end_ptr++;
				while (*end_ptr == '\t')
					end_ptr++;
				while (*end_ptr == ' ')
					end_ptr++;
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
		} else
		{
			if (*(++start_ptr) == '\t' || *start_ptr == ' ')
			{
				*(ptr++) = ' ';
				wbytes++;
				while (*start_ptr == '\t')
					start_ptr++;
				while (*start_ptr == ' ')
					start_ptr++;
				while (*start_ptr == '\t')
					start_ptr++;
				while (*start_ptr == ' ')
					start_ptr++;
				start_ptr--;
			} else
				break;
		}
	}
	chunk[wbytes] = 0;

	/* Decode */
	if ((tmp = c2_str_decode_iso_8859_1 (chunk)))
	{
		g_free (chunk);
		chunk = tmp;
	}
	
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

gchar *
create_boundary (void)
{
	gchar *boundary;
	gchar *ptr;
	gint i;
	
	srand (time (NULL));
	boundary = g_new0 (gchar, BOUNDARY_LENGTH);
	sprintf (boundary, "Cronos-II=");
	ptr = boundary+10;
	
	for (i = 0; i < BOUNDARY_LENGTH-11; i++) 
		*(ptr+i) = (rand () % 26)+97; /* From a to z */
	
	if (*(ptr+i-1) == '-')
		*(ptr+i-1) = '.';
	
	*(ptr+i) = '\0';
	
	return boundary;
}

C2Message *
c2_message_fix_broken_message (C2Message *message)
{
	C2Message *fmessage;
	C2Mime *mime;
	gchar *buf, *ptr;
	gchar *tmpfile;
	gchar *boundary = NULL;
	FILE *fd;
	
	c2_return_val_if_fail (C2_IS_MESSAGE (message), NULL, C2EDATA);

	/* Get a tmp file */
	tmpfile = c2_get_tmp_file (NULL);

	/* Open the tmp file */
	if (!(fd = fopen (tmpfile, "w")))
	{
		c2_error_set (-errno);
		g_free (tmpfile);
		return NULL;
	}

	buf = c2_message_get_header_field (message, "Content-Type:");
	
	if (c2_strneq (buf, "multipart", 9))
	{
		/* Try to get the boundary */
		boundary = strstr (buf, "boundary=");
		if (boundary)
		{
			boundary += 9;
			ptr = g_strdup (boundary);
			boundary = c2_str_strip_enclosed (ptr, '"', '"');
			g_free (ptr);
		}
	} else
		/* We just point it somewhere so we don't
		 * get confused later
		 */
		boundary = message->header;

	g_free (buf);

	for (ptr = message->header; ptr;)
	{
		gchar *ptrn = strchr (ptr, '\n');
		gint length = ptrn ? (ptrn - ptr) : strlen (ptr);
		gboolean finish = ptrn ? FALSE : TRUE;

		fwrite (ptr, sizeof (gchar), length, fd);
			
		/* Check if this is the Content-Type header */
		if (c2_strneq (ptr, "Content-Type:", 13) && !boundary)
		{
			/* It's not, finish the line and continue */
			fwrite ("; boundary=\"", sizeof (gchar), 12, fd);
			boundary = create_boundary ();
			fwrite (boundary, sizeof (gchar), BOUNDARY_LENGTH-1, fd);
			fwrite ("\"", sizeof (gchar), 1, fd);
			
		}
		
		fwrite ("\n", sizeof (gchar), 1, fd);
		ptr += length+1;

		if (finish)
			break;
	}

	/* Write the separator */
//	fwrite ("\n", sizeof (gchar), 1, fd); I'm getting an extra '\n'...

	/* Now work over the body */
	if (c2_mime_length (message->mime) > 1)
	{
		/* Write the MIME_UNCAPABLE_WARNING */
		fprintf (fd, MIME_UNCAPABLE_WARNING "\n\n");
	
		for (mime = message->mime; mime; mime = mime->next)
		{
			gint len;
			gboolean free_space = TRUE;

			if (mime->part)
				free_space = FALSE;
			else
			{
#ifdef USE_DEBUG
				g_error ("The thing you thought would never get true "
						 "got true, so see what's going on!\n");
#endif
				c2_mime_get_part (mime);
			}

			/* Encode the part */
			if (c2_streq (mime->encoding, "7bit") || c2_streq (mime->encoding, "8bit"))
			{
				buf = g_strdup (mime->part);

				if (buf)
					len = strlen (buf);
			} else if (c2_streq (mime->encoding, "base64"))
			{
encode_base64:
				len = mime->length;
				buf = c2_mime_encode_base64 (mime->part, &len);
			} else
			{
				/* [TODO] Add more supports */
#ifdef USE_DEBUG
				g_print ("There is no support for encoding in type %s (%s, %d)\n",
							mime->encoding, __FILE__, __LINE__);
#endif
				mime->encoding = g_strdup ("base64");
				goto encode_base64;
			}
			
			fprintf (fd, "--%s\n"
						 "Content-Type: %s/%s\n"
						 "Content-Transfer-Encoding: %s\n",
						 boundary, mime->type, mime->subtype, mime->encoding);

			if (mime->disposition && strlen (mime->disposition))
				fprintf (fd, "Content-Disposition: %s\n",
								mime->disposition);

			if (mime->description && strlen (mime->description))
				fprintf (fd, "Content-Description: %s\n",
								mime->description);

			fprintf (fd, "\n");
			fwrite (buf, sizeof (gchar), len, fd);
			/* fprintf (fd, "\n"); I'm getting an extra space */

			g_free (buf);

			if (free_space)
			{
				g_free (mime->part);
				mime->part = NULL;
			}
		}

		fprintf (fd, "--%s--", boundary);
	} else if (message->mime)
	{
		/* This is just one part */
		fprintf (fd, "%s", message->mime->part);
	} else
	{
		/* No attachments, just write what we
		 * have in body
		 */
		fprintf (fd, "%s", message->body);
	}

	//return message;
	
	/* Close the tmpfile */
	fclose (fd);

	/* Now, load the message and return it */
	fmessage = c2_db_message_get_from_file (tmpfile);

	/* And now delete the file */
	unlink (tmpfile);
	g_free (tmpfile);

	return fmessage;
}
