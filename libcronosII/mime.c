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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <glib.h>
#include <string.h>

#include "error.h"
#include "i18n.h"
#include "mime.h"
#include "message.h"
#include "utils.h"

static void
c2_mime_class_init							(C2MimeClass *klass);

static void
c2_mime_init								(C2Mime *mime);

static void
c2_mime_destroy								(GtkObject *object);

static void
parse_content_type							(const gchar *content_type, gchar **type,
											 gchar **subtype, gchar **parameter);

static gchar *
get_parameter_value							(const gchar *parameter, const gchar *field);

static C2Mime *
get_mime_parts								(const gchar *body, const gchar *boundary,
											 const gchar *parent_boundary);

enum
{
	LAST_SIGNAL
};

static guint c2_mime_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

void
c2_mime_construct (C2Mime **head, C2Message *message)
{
	gchar *mime_version;
	gchar *content_type, *type, *subtype, *parameter, *boundary;
	gchar *buf;
	
	c2_return_if_fail (message, C2EDATA);

	/* First check for the field MIME-Version */
	mime_version = c2_message_get_header_field (message, "MIME-Version:");

	if (!mime_version)
	{
no_mime_information:
		/* This is most likely to be a plain/text message in standard rfc822 encoding */
		*head = c2_mime_new (NULL);
		(*head)->start = c2_message_get_message_body (message);
		(*head)->length = strlen ((*head)->start);
		(*head)->part = NULL;
		(*head)->type = g_strdup ("text");
		(*head)->subtype = g_strdup ("plain");
		(*head)->id = NULL;
		(*head)->parameter = NULL;
		(*head)->encoding = g_strdup ("7bit");
		
		g_free (mime_version);
		return;
	}
#ifdef USE_DEBUG
	if (c2_strne (mime_version, "1.0"))
		/* This is a format different than "1.0" that is not defined
		 * when this is being written (10.5.2001).
		 * We will try to understand it anyway.
		 */
		g_print ("The selected message uses an unknown version of MIME encoding: %s\n", mime_version);
#endif
	g_free (mime_version);

	/* Get the Content-Type */
	content_type = c2_message_get_header_field (message, "Content-Type:");
	
	if (!content_type)
	{
		type = g_strdup ("text");
		subtype = g_strdup ("plain");
		parameter = NULL;
	} else
	{
		parse_content_type (content_type, &type, &subtype, &parameter);
		g_free (content_type);
	}

	if (c2_streq (type, "multipart"))
	{
		if (!parameter)
		{
			g_warning (_("This message claims to be multipart, but is broken."));
			goto no_mime_information;
		}

		if (!(boundary = get_parameter_value (parameter, "boundary")))
		{
			g_warning (_("This message claims to be multipart, but is broken."));
			goto no_mime_information;
		}

		*head = get_mime_parts (message->body, boundary, NULL);
	} else 
	{
		*head = c2_mime_new (NULL);
		(*head)->start = c2_message_get_message_body (message);
		(*head)->length = strlen ((*head)->start);
		(*head)->part = NULL;
		(*head)->type = type;
		(*head)->subtype = subtype;
		(*head)->parameter = parameter;
		(*head)->disposition = c2_message_get_header_field (message, "Content-Disposition:");
		(*head)->encoding = c2_message_get_header_field (message, "Content-Transfer-Encoding:");
		buf = c2_message_get_header_field (message, "\nContent-ID:");
		if (buf)
			(*head)->id = c2_str_strip_enclosed (buf, '<', '>');
		else
			(*head)->id = NULL;
		g_free (buf);
	}
}

static void
parse_content_type (const gchar *content_type, gchar **type, gchar **subtype, gchar **parameter)
{
	const gchar *ptr1, *ptr2;
	
	*type = *subtype = *parameter = NULL;

	c2_return_if_fail (content_type, C2EDATA);

	/* Type */
	for (ptr1 = ptr2 = content_type; *ptr2 != '\0' && *ptr2 != '/'; ptr2++)
		;
	*type = g_strndup (ptr1, ptr2-ptr1);

	/* Subtype */
	for (ptr1 = ++ptr2; *ptr2 != '\0' && *ptr2 != ';'; ptr2++)
		;
	if (*ptr2 != '\0')
		*subtype = g_strndup (ptr1, ptr2-ptr1);
	else
	{
		*subtype = g_strdup (ptr1);
		return;
	}
	
	/* Parameter */
	for (ptr1 = ++ptr2; *ptr1 == ' '; ptr1++)
		;
	if (*ptr1 != '\0')
		*parameter = g_strdup (ptr1);
}

static gchar *
get_parameter_value (const gchar *parameter, const gchar *field)
{
	const gchar *ptr;

	if (!(ptr = c2_strstr_case_insensitive (parameter, field)))
		return NULL;

	ptr += strlen (field);

	while (*ptr == '=' && *ptr != '\0')
		ptr++;

	if (!ptr)
		return NULL;

	return c2_str_strip_enclosed (ptr, '"', '"');
}

static C2Mime *
get_mime_parts (const gchar *body, const gchar *boundary, const gchar *parent_boundary)
{
	C2Mime *head = NULL, *mime = NULL, *child = NULL;
	gchar *content_type = NULL, *type = NULL, *subtype = NULL, *parameter = NULL;
	gchar *local_boundary;
	gchar *buf;
	gint blength = strlen (boundary);
	const gchar *ptr, *end;
	gboolean end_reached = FALSE;

	for (ptr = strstr (body, boundary); !end_reached && ptr;)
	{
		/* Calculate the end of the part */
		end = strstr (ptr+blength, boundary);

		/* Check if this end is the end of the multipart (parent) type */
		if (end && c2_strneq (end+blength, "--", 2))
			end_reached = TRUE;

		if (!end)
		{
			/* Check if the reason there's no end boundary is that
			 * we are a sub multipart message (recursive).
			 */
			if (parent_boundary)
				if ((end = strstr (ptr+blength, parent_boundary)))
					end_reached = TRUE;
		}

		/* Get the content type of this part */
		if ((content_type = c2_message_str_get_header_field (ptr, "Content-Type:")))
			parse_content_type (content_type, &type, &subtype, &parameter);
		g_free (content_type);

		if (c2_streq (type, "multipart"))
		{
			gchar *pos;

			if (!parameter)
			{
				g_warning (_("This message claims to be multipart but it seems to be broken."));
				break;
			}

			if (!(local_boundary = get_parameter_value (parameter, "boundary")))
			{
				g_warning (_("This message claims to be multipart but it seems to be broken."));
				break;
			}

			pos = strstr (ptr, "\n\n") + 2;

			child = get_mime_parts (pos, local_boundary, boundary);
			head = c2_mime_append (head, child);
			g_free (parameter);
			g_free (type);
			g_free (subtype);
		} else
		{
			mime = C2_MIME (c2_mime_new (NULL));
			mime->start = strstr (ptr, "\n\n") + 2;
			if (end)
				mime->length = end - mime->start - 3; /* 3 = '\n--' */
			else
				mime->length = strlen (mime->start);
			mime->part = NULL;

			mime->type = type;
			mime->subtype = subtype;
			mime->parameter = parameter;

			buf = c2_message_str_get_header_field (ptr, "Content-ID:");
			
			if (buf)
				mime->id = c2_str_strip_enclosed (buf, '<', '>');
			else
				mime->id = NULL;
			g_free (buf);

			mime->disposition = c2_message_str_get_header_field (ptr, "Content-Disposition:");
			mime->encoding = c2_message_str_get_header_field (ptr, "Content-Transfer-Encoding:");
			head = c2_mime_append (head, mime);
		}

		ptr = end;
	}

	return head;
}

C2Mime *
c2_mime_append (C2Mime *head, C2Mime *mime)
{
	C2Mime *ptr;

	c2_return_val_if_fail (mime, head, C2EDATA);
	
	if (!head)
		return mime;

	for (ptr = head; ptr->next; ptr = ptr->next)
		;

	ptr->next = mime;
	mime->previous = ptr;
	
	return head;
}

guint
c2_mime_get_type (void)
{
	static guint c2_mime_type = 0;

	if (!c2_mime_type)
	{
		GtkTypeInfo c2_mime_info = {
			"C2Mime",
			sizeof (C2Mime),
			sizeof (C2MimeClass),
			(GtkClassInitFunc) c2_mime_class_init,
			(GtkObjectInitFunc) c2_mime_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_mime_type = gtk_type_unique (gtk_object_get_type (), &c2_mime_info);
	}

	return c2_mime_type;
}

static void
c2_mime_class_init (C2MimeClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	gtk_object_class_add_signals (object_class, c2_mime_signals, LAST_SIGNAL);

	object_class->destroy = c2_mime_destroy;
}

static void
c2_mime_init (C2Mime *mime)
{
	mime->start = NULL;
	mime->length = 0;
	mime->part = NULL;
	mime->type = NULL;
	mime->subtype = NULL;
	mime->id = NULL;
	mime->parameter = NULL;
	mime->disposition = NULL;
	mime->encoding = NULL;
	mime->previous = NULL;
	mime->next = NULL;
}

static void
c2_mime_destroy (GtkObject *object)
{
	C2Mime *mime, *l;

	c2_return_if_fail (C2_IS_MIME (object), C2EDATA);

	mime = C2_MIME (object);
	
	if (!mime->previous)
	{
		for (l = mime; l != NULL;)
		{
			mime = l->next;
			gtk_object_unref (GTK_OBJECT (l));
			l = mime;
		}
	} else
	{
		g_free (mime->part);
		g_free (mime->type);
		g_free (mime->subtype);
		g_free (mime->id);
		g_free (mime->parameter);
		g_free (mime->disposition);
		g_free (mime->encoding);
	}
}

C2Mime *
c2_mime_new (C2Message *message)
{
	C2Mime *mime;
	
	if (!message)
		return gtk_type_new (c2_mime_get_type ());

	if (message->mime)
		return message->mime;

	c2_mime_construct (&mime, message);

	return mime;
}

const gchar *
c2_mime_get_part (C2Mime *mime)
{
	gchar *tmp;
	
	if (mime->part)
		return mime->part;

	tmp = g_strndup (mime->start, mime->length);

	if (c2_streq (mime->encoding, "base64"))
	{
		mime->part = c2_mime_decode_base64 (tmp, &mime->length);
		g_free (tmp);
	} else if (c2_streq (mime->encoding, "quoted-printable"))
	{
		mime->part = c2_mime_decode_quoted_printable (tmp, &mime->length);
		g_free (tmp);
	} else
		mime->part = tmp;

	return mime->part;
}

C2Mime *
c2_mime_get_part_by_content_type (C2Mime *mime, const gchar *content_type)
{
	C2Mime *l;
	gchar *type, *subtype, *parameter;

	parse_content_type (content_type, &type, &subtype, &parameter);

	for (l = mime; l != NULL; l = l->next)
	{
		if (c2_streq (l->type, type) &&
			c2_streq (l->subtype, subtype))
			break;
	}

	if (!l)	
		if ((l = mime))
			return;

	c2_mime_get_part (l);

	g_free (type);
	g_free (subtype);
	g_free (parameter);

	return l;
}

static gchar base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/**
 * c2_mime_encode_base64
 * @data: A pointer to the part.
 * @length: A pointer to the length of the part.
 *
 * Encodes a part in base64.
 *
 * Return Value:
 * The encoded part.
 **/
gchar *
c2_mime_encode_base64 (gchar *data, gint *length)
{
	gchar *encoded, *index, buffer[3];
	gint pos, len;
	
	/* Invalid inputs will cause this function to crash.  Return an error. */
	if (!length || !data)
		return 0;
	
	encoded = g_malloc0(sizeof(gchar)*(gint)(*length * 1.40)); /* it gets 33% larger */
	pos = 0;
	len = 0;
	index = data;
	while (index-data < *length)
	{
		if ( index-data+3 <= *length )
		{
			memcpy (buffer, index, 3);
			*(encoded+pos)   = base64_chars[(buffer[0] >> 2) & 0x3f];
			*(encoded+pos+1) = base64_chars[((buffer[0] << 4) & 0x30) | ((buffer[1] >> 4) & 0xf)];
			*(encoded+pos+2) = base64_chars[((buffer[1] << 2) & 0x3c) | ((buffer[2] >> 6) & 0x3)];
			*(encoded+pos+3) = base64_chars[buffer[2] & 0x3f];
		} else if (index-data+2 == *length)
		{
			memcpy(buffer, index, 2);
			*(encoded+pos)   = base64_chars[(buffer[0] >> 2) & 0x3f];
			*(encoded+pos+1) = base64_chars[((buffer[0] << 4) & 0x30) | ((buffer[1] >> 4) & 0xf)];
			*(encoded+pos+2) = base64_chars[(buffer[1] << 2) & 0x3c];
			*(encoded+pos+3) = '=';
		} else if (index-data+1 == *length)
		{
			memcpy(buffer, index, 1);
			*(encoded+pos)   = base64_chars[(buffer[0] >> 2) & 0x3f];
			*(encoded+pos+1) = base64_chars[(buffer[0] << 4) & 0x30];
			*(encoded+pos+2) = '=';
			*(encoded+pos+3) = '=';
		} else
		{
			g_error ("encode_base64(): corrupt data");
			return NULL;
		}
		
		len += 4;
		
		/* base64 can only have 76 chars per line */
		if (len >= 76)
		{
			*(encoded + pos + 4) = '\n';
			pos++;
			len = 0;
		}
		
		pos += 4;
		index += 3;
	}
	
	/* if there were less then a full triplet left, we pad the remaining
	 * encoded bytes with = */
	/*
	 * if (*length % 3 == 1)
	 * {
	 *     *(encoded+pos-1) = '=';
	 *     *(encoded+pos-2) = '=';
	 * }
	 * if (*length % 3 == 2)
	 * {
	 *     *(encoded+pos-1) = '=';
	 * }*/
	
	*(encoded+pos) = '\n';
	*(encoded+pos+1) = '\0';
	
	*length = strlen(encoded);
	
	return encoded;
}

/**
 * c2_mime_decode_base64
 * @data: A pointer to the encoded part.
 * @length: A pointer to the length of the encoded part.
 *
 * Decodes a part in base64.
 *
 * Return Value:
 * The decoded part.
 **/
gchar *
c2_mime_decode_base64 (gchar *data, gint *length)
{
	/* This function is based in decode_base64 by Jeffrey Stedfast */
	gchar *output, *workspace, *p;
	gulong pos = 0;
	gint i, a[4], len = 0;
	
	c2_return_val_if_fail (data, NULL, C2EDATA);
	
	workspace = (gchar *) g_malloc0 (sizeof (gchar) * ((gint)(strlen(data) / 1.33) + 2));
	
	while (*data && len < *length)
	{
		for (i = 0; i < 4; i++, data++, len++)
		{
			if ((p = strchr (base64_chars, *data)))
				a[i] = (gint)(p - base64_chars);
			else
				i--;
		}
		
		workspace[pos]     = (((a[0] << 2) & 0xfc) | ((a[1] >> 4) & 0x03));
		workspace[pos + 1] = (((a[1] << 4) & 0xf0) | ((a[2] >> 2) & 0x0f));
		workspace[pos + 2] = (((a[2] << 6) & 0xc0) | (a[3] & 0x3f));
		
		if (a[2] == 64 && a[3] == 64)
		{
			workspace[pos + 1] = 0;
			pos -= 2;
		} else
		{
			if (a[3] == 64)
			{
				workspace[pos + 2] = 0;
				pos--;
			}
		}
		pos += 3;
	}
	
	output = g_malloc0 (pos + 1);
	memcpy (output, workspace, pos);
	
	*length = pos;
	
	g_free (workspace);
	
	return output;
}

/**
 * c2_mime_decode_quoted_printable
 * @data: A pointer to the encoded part.
 * @length: A pointer to the length of the encoded part.
 *
 * Decodes a part in quoted_printable.
 *
 * Return Value:
 * The decoded part.
 **/
gchar *
c2_mime_decode_quoted_printable (gchar *message, gint *length)
{
	/* This function is based in decode_quoted_printable by Jeffrey Stedfast */
	gchar *buffer, *index, ch[2];
	gint i = 0, temp;
	
	buffer = g_malloc0 (*length + 1);
	index = message;
	
	while (index - message < *length)
	{
		if (*index == '=')
		{
			index++;
			if (*index != '\n')
			{
				sscanf (index, "%2x", &temp);
				sprintf (ch, "%c", temp);
				buffer[i] = ch[0];
			} else
				buffer[i] = index[1];
			i++;
			index += 2;
		} else
		{
			buffer[i] = *index;
			i++;
			index++;
		}
	}
	buffer[i] = '\0';
	
	*length = strlen (buffer);
	return buffer;
}
