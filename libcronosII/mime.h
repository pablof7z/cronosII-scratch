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
#ifndef __LIBCRONOSII_MIME_H__
#define __LIBCRONOSII_MIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <gtk/gtkobject.h>

#define C2_MIME(obj)							GTK_CHECK_CAST (obj, c2_mime_get_type (), C2Mime)
#define C2_MIME_CLASS(klass)					GTK_CHECK_CLASS_CAST (klass, c2_mime_get_type (), C2MimeClass)
#define C2_IS_MIME(obj)							GTK_CHECK_TYPE (obj, c2_mime_get_type ())

typedef struct _C2Mime C2Mime;
typedef struct _C2MimeClass C2MimeClass;

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "message.h"
#else
#	include <cronosII.h>
#endif

struct _C2Mime
{
	GtkObject object;
	
	const gchar *start;
	guint length;

	gchar *part;

	gchar *type, *subtype, *id;
	gchar *parameter;
	gchar *disposition;
	gchar *description;
	gchar *encoding;

	C2Mime *previous;
	C2Mime *next;
};

struct _C2MimeClass
{
	GtkObjectClass parent_class;
};

guint
c2_mime_get_type							(void);

C2Mime *
c2_mime_new									(C2Message *message);

void
c2_mime_construct							(C2Mime **head, C2Message *message);

gint
c2_mime_length								(C2Mime *mime);

C2Mime *
c2_mime_append								(C2Mime *head, C2Mime *mime);

const gchar *
c2_mime_get_part							(C2Mime *mime);

C2Mime *
c2_mime_get_part_by_content_type			(C2Mime *mime, const gchar *content_type);

gchar *
c2_mime_get_parameter_value					(const gchar *parameter, const gchar *field);

gchar *
c2_mime_encode_base64						(gchar *data, gint *length);

gchar *
c2_mime_decode_base64						(gchar *data, gint *length);

gchar *
c2_mime_decode_quoted_printable				(gchar *message, gint *length);

#ifdef __cplusplus
}
#endif

#endif
