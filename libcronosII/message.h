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
#ifndef __LIBCRONOSII_MESSAGE_H__
#define __LIBCRONOSII_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <time.h>

#define C2_TYPE_MESSAGE						(c2_message_get_type ())
#define C2_MESSAGE(obj)						(GTK_CHECK_CAST (obj, C2_TYPE_MESSAGE, C2Message))
#define C2_MESSAGE_CLASS(klass)				(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_MESSAGE, C2MessageClass))
#define C2_IS_MESSAGE(obj)					(GTK_CHECK_TYPE(obj, C2_TYPE_MESSAGE))
#define C2_IS_MESSAGE_CLASS(klass)			(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_MESSAGE))

typedef struct _C2Message C2Message;
typedef struct _C2MessageClass C2MessageClass;
typedef enum _C2MessageState C2MessageState;
typedef enum _C2MessageAction C2MessageAction;

#ifdef BUILDING_C2
#	include "mime.h"
#else
#	include <cronosII.h>
#endif

enum _C2MessageState
{
	C2_MESSAGE_READED		= 1,
	C2_MESSAGE_UNREADED		= 2,
	C2_MESSAGE_REPLIED		= 3,
	C2_MESSAGE_FORWARDED	= 4
};

enum _C2MessageAction
{
	C2_MESSAGE_DELETE,
	C2_MESSAGE_EXPUNGE,
	C2_MESSAGE_MOVE,
	C2_MESSAGE_COPY
};

struct _C2Message
{
	GtkObject object;

	gchar *header;
	gchar *body;
	
	C2Mime *mime;
};

struct _C2MessageClass
{
	GtkObjectClass parent_class;
	
	void (*message_die) (C2Message *message);
};

GtkType
c2_message_get_type									(void);

C2Message *
c2_message_new										(void);

void
c2_message_set_message								(C2Message *message, const gchar *string);

const gchar *
c2_message_get_message_header						(const C2Message *message);

const gchar *
c2_message_get_message_body							(const C2Message *message);

gchar *
c2_message_get_header_field							(C2Message *message, const gchar *field);

gchar *
c2_message_str_get_header_field						(const gchar *message, const gchar *field);

#ifdef __cplusplus
}
#endif

#endif
