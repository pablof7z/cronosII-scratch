/*  Cronos II Mail Client
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
#include <config.h>
#include <pthread.h>

#include <libcronosII/error.h>
#include <libcronosII/request.h>
#include <libcronosII/utils.h>

#include "widget-part.h"

static void
c2_part_class_init								(C2PartClass *klass);

static void
c2_part_init									(C2Part *part);

enum
{
	LAST_SIGNAL
};

static gint c2_part_signals[LAST_SIGNAL] = { 0 };

static C2HtmlClass *parent_class = NULL;

guint
c2_part_get_type (void)
{
	static guint c2_part_type = 0;

	if (!c2_part_type)
	{
		GtkTypeInfo c2_part_info =
		{
			"C2Part",
			sizeof (C2Part),
			sizeof (C2PartClass),
			(GtkClassInitFunc) c2_part_class_init,
			(GtkObjectInitFunc) c2_part_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		c2_part_type = gtk_type_unique (c2_html_get_type (), &c2_part_info);
	}
	
	return c2_part_type;
}

static void
c2_part_class_init (C2PartClass *klass)
{
	parent_class = gtk_type_class (c2_html_get_type ());
}

static void
c2_part_init (C2Part *part)
{
}

GtkWidget *
c2_part_new (void)
{
	C2Part *part;
	
	part = gtk_type_new (c2_part_get_type ());
	return GTK_WIDGET (part);
}

void
c2_part_set_part (C2Part *part, C2Mime *mime)
{
	c2_return_if_fail (mime, C2EDATA);

#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
	gtk_xmhtml_freeze (GTK_XMHTML (part));
	gtk_xmhtml_source (GTK_XMHTML (part), c2_mime_get_part (mime));
	gtk_xmhtml_thaw (GTK_XMHTML (part));
#else
#endif
}
