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
#include <gnome.h>

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "widget-HTML.h"
#include "widget-mail.h"
#include "widget-part.h"

static void
on_body_button_press_event					(GtkWidget *widget, GdkEventButton *event);

static void
c2_mail_class_init							(C2MailClass *klass);

static void
c2_mail_init								(C2Mail *mail);

void
html_link_manager_cid						(C2Html *html, const gchar *url, GtkHTMLStream *stream);

static gchar *
interpret_text_plain_symbols				(const gchar *plain);

static gchar *
get_word									(const gchar *cptr, gchar **extra);

enum
{
	LAST_SIGNAL
};

static gint c2_mail_signals[LAST_SIGNAL] = { 0 };

void
c2_mail_set_message (C2Mail *mail, C2Message *message)
{
	C2Mime *mime;
	gboolean text_plain = TRUE;
	gchar *string;

	c2_return_if_fail (message, C2EDATA);

	/* Get the part that should be displayed */
	mail->application->options_default_mime = 1;
	string = message->body;
	
	switch (mail->application->options_default_mime)
	{
		case C2_DEFAULT_MIME_PLAIN:
			if ((mime = c2_mime_get_part_by_content_type (message->mime, "text/plain")))
				string = mime->part;
			break;
		case C2_DEFAULT_MIME_HTML:
			if (!(mime = c2_mime_get_part_by_content_type (message->mime, "text/htm")))
			{
				if ((mime = c2_mime_get_part_by_content_type (message->mime, "text/plain")))
					string = mime->part;
			} else
			{
				string = mime->part;
				text_plain = FALSE;
			}
						

			break;
		default:
			if ((mime = c2_mime_get_part_by_content_type (message->mime, "text/plain")))
				string = mime->part;
			break;
	}

	if (text_plain)
	{
		string = interpret_text_plain_symbols (mime->part);
//		string = g_strdup_printf ("<HTML><BODY BGCOLOR=#ffffff><TT>%s</TT></BODY></HTML>", mime->part);
	}

	gtk_object_set_data (GTK_OBJECT (mail->body), "message", message);
	
	c2_html_set_content_from_string (C2_HTML (mail->body), string);

	if (text_plain)
		g_free (string);
}

void
c2_mail_construct (C2Mail *mail, C2Application *application)
{
	GtkWidget *parent;
	GtkWidget *scroll;

	mail->application = application;

	mail->table = gtk_table_new (5, 4, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (mail), mail->table, FALSE, FALSE, 0);
	gtk_widget_show (mail->table);
	
#ifdef USE_GTKHTML
	scroll = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (mail), scroll, TRUE, TRUE, 0);
	gtk_widget_show (scroll);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (scroll), GTK_SHADOW_IN);
	
	parent = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scroll), parent);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
#elif defined (USE_GTKXMHTML)
	parent = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (mail), parent, TRUE, TRUE, 0);
#else
	parent = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (GTK_BOX (mail), parent, TRUE, TRUE, 0);
#endif
	gtk_widget_show (parent);

	mail->body = c2_html_new ();
	gtk_container_add (GTK_CONTAINER (parent), mail->body);
	gtk_widget_show (mail->body);
#ifdef USE_GTKHTML
	c2_html_set_link_manager (C2_HTML (mail->body), "cid", html_link_manager_cid);
#endif
}

GtkWidget *
c2_mail_new (C2Application *application)
{
	C2Mail *mail;
	mail = gtk_type_new (c2_mail_get_type ());
	c2_mail_construct (mail, application);
	return GTK_WIDGET (mail);
}

void
c2_mail_install_hints (C2Mail *mail, GtkWidget *appbar, pthread_mutex_t *lock)
{
	c2_html_install_hints (C2_HTML (mail->body), appbar, lock);
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

void
html_link_manager_cid (C2Html *html, const gchar *url, GtkHTMLStream *stream)
{
	C2Message *message = C2_MESSAGE (gtk_object_get_data (GTK_OBJECT (html), "message"));
	C2Mime *mime;

	if (!message)
	{
		g_print ("Internal error, unable to get message from Widget.\n");
		return;
	}

	for (mime = message->mime; mime; mime = mime->next)
		if (c2_streq (mime->id, url+4))
			break;

	if (!mime)
	{
		L
		return;
	}
	
	gtk_html_stream_write (stream, c2_mime_get_part (mime), mime->length);
}

/* [TODO]
 * Can this be optimized as we usually do?
 */
static gchar *
interpret_text_plain_symbols (const gchar *plain)
{
	GString *string = g_string_new ("<HTML><BODY BGCOLOR=#ffffff><PRE>");
	const gchar *ptr;
	gchar *word, *extra, *buf;
	gboolean quoted = FALSE;
	gint length;
	
	for (ptr = plain;;)
	{
		word = get_word (ptr, &extra);
		if (!strlen (word) && !strlen (extra))
			break;
		
		g_string_append (string, extra);
		length = strlen (word);
		ptr += length+strlen (extra);
		g_free (extra);

		/* First do a simple check if we have to act in this word. */
		if (word[0] == word[length-1])
		{
			if (word[0] == '_')
			{
				gchar *buf2 = c2_str_strip_enclosed (word, '_', '_');
				
				buf = g_strdup_printf ("<u>%s</u>", buf2);
				g_free (buf2);
			} else if (word[0] == '*')
			{
				gchar *buf2 = c2_str_strip_enclosed (word, '*', '*');
				
				buf = g_strdup_printf ("<b>%s</b>", buf2);
				g_free (buf2);
			}
			/* Italic: What's the text/plain symbol???
			 * [TODO] Ask Pete, he probably knows... */
			else
				buf = g_strdup (word);
			
			g_string_append (string, buf);
			g_free (buf);
		} else
		{
			if (c2_streq (word, ":)"))
				g_string_append (string, "<IMG SRC=\"c2dist://html-icons/:).png\" WIDTH=10 HEIGHT=10>");
			else if (c2_streq (word, ":D"))
				g_string_append (string, "<IMG SRC=\"c2dist://html-icons/:D.png\" WIDTH=10 HEIGHT=10>");
			else
				g_string_append (string, word);
		}

		g_free (word);
	}

	g_string_append (string, "</PRE></BODY></HTML>");

	buf = string->str;
	g_string_free (string, FALSE);

	return buf;
}

static gchar *
get_word (const gchar *cptr, gchar **extra)
{
	GString *sextra = g_string_new (NULL);
	const gchar *ptr, *end;
	gchar *word;

	for (ptr = cptr; ptr &&
				(ptr[0] == ' ' ||
				 ptr[0] == '\t' ||
				 ptr[0] == '\n'); ptr++)
		g_string_append_c (sextra, ptr[0]);
	
	for (end = ptr; end &&
				(end[0] != ' ' &&
				 end[0] != '\t' &&
				 end[0] != '\n'); end++)
		;

	if (end)
		word = g_strndup (ptr, end-ptr);
	else
		word = g_strdup (ptr);

	*extra = sextra->str;
	
	g_string_free (sextra, FALSE);

	return word;
}
