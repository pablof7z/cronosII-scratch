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

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "widget-mail.h"
#include "widget-part.h"

#include "c2-app.h"

static void
on_body_button_press_event						(GtkWidget *widget, GdkEventButton *event);

static void
c2_mail_class_init								(C2MailClass *klass);

static void
c2_mail_init									(C2Mail *mail);

enum
{
	LAST_SIGNAL
};

static gint c2_mail_signals[LAST_SIGNAL] = { 0 };

void
c2_mail_set_message (C2Mail *mail, C2Message *message)
{
	C2Mime *mime;

	c2_return_if_fail (message, C2EDATA);

	/* Get the part that should be displayed */
	switch (c2_app.options_default_mime)
	{
		case C2_DEFAULT_MIME_PLAIN:
			mime = c2_mime_get_part_by_content_type (message->mime, "text/plain");
			break;
		case C2_DEFAULT_MIME_HTML:
			if (!(mime = c2_mime_get_part_by_content_type (message->mime, "text/html")))
				mime = c2_mime_get_part_by_content_type (message->mime, "text/plain");
			break;
		default:
			mime = c2_mime_get_part_by_content_type (message->mime, "text/plain");
			break;
	}

	c2_part_set_part (C2_PART (mail->body), mime);
}

void
c2_mail_construct (C2Mail *mail)
{
	GtkWidget *parent;
	GtkWidget *scroll;

	mail->table = gtk_table_new (5, 4, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (mail), mail->table, FALSE, FALSE, 0);
	gtk_widget_show (mail->table);
	
#ifdef USE_GTKHTML
#elif defined (USE_GTKXMHTML)
	parent = gtk_viewport_new (NULL, NULL);
#else
#endif
	gtk_box_pack_start (GTK_BOX (mail), parent, TRUE, TRUE, 0);
	gtk_widget_show (parent);
	
	mail->body = c2_part_new ();
	gtk_container_add (GTK_CONTAINER (parent), mail->body);
	gtk_widget_show (mail->body);
}

GtkWidget *
c2_mail_new (void)
{
	C2Mail *mail;
	mail = gtk_type_new (c2_mail_get_type ());
	c2_mail_construct (mail);
	return GTK_WIDGET (mail);
}

void
c2_mail_install_hints (C2Mail *mail, GtkWidget *appbar)
{
	c2_part_install_hints (C2_PART (mail->body), appbar);
}

static void
c2_mail_destroy (GtkObject *object)
{
	if (C2_MAIL (GTK_WIDGET (object))->message)
		gtk_object_unref (GTK_OBJECT (C2_MAIL (GTK_WIDGET (object))->message));
}

static void
on_body_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
}

guint
c2_mail_get_type (void)
{
	static guint c2_mail_type = 0;

	if (!c2_mail_type)
	{
		GtkTypeInfo c2_mail_info =
		{
			"C2Mail",
			sizeof (C2Mail),
			sizeof (C2MailClass),
			(GtkClassInitFunc) c2_mail_class_init,
			(GtkObjectInitFunc) c2_mail_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		c2_mail_type = gtk_type_unique (gtk_vbox_get_type (), &c2_mail_info);
	}

	return c2_mail_type;
}

static void
c2_mail_class_init (C2MailClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	container_class = (GtkContainerClass *) klass;

	gtk_object_class_add_signals (object_class, c2_mail_signals, LAST_SIGNAL);

	object_class->destroy = c2_mail_destroy;
}

static void
c2_mail_init (C2Mail *mail)
{
	mail->message			= NULL;
	mail->showing_from		= 0;
	mail->showing_to		= 0;
	mail->showing_cc		= 0;
	mail->showing_bcc		= 0;
	mail->showing_subject	= 0;
	mail->showing_priority	= 0;
}
