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
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <libcronosII/error.h>

#include "widget-window.h"

static void
class_init									(C2WindowClass *klass);

static void
init										(C2Window *window);

static void
destroy										(GtkObject *object);

static gint
on_key_press_event							(GtkWidget *widget, GdkEventKey *event);

static gint
on_report_timeout							(gpointer data);

static gint
on_activity_timeout							(gpointer data);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GnomeAppClass *parent_class = NULL;

GtkType
c2_window_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2Window",
			sizeof (C2Window),
			sizeof (C2WindowClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gnome_app_get_type (), &info);
	}

	return type;
}

static void
class_init (C2WindowClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;
	
	parent_class = gtk_type_class (gnome_app_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	widget_class->key_press_event = on_key_press_event;
}

static void
init (C2Window *window)
{
	window->application = NULL;
	window->xml = NULL;
	c2_mutex_init (&window->status_lock);
	c2_mutex_init (&window->progress_lock);
}

static void
destroy (GtkObject *object)
{
	C2Window *window;

	window = C2_WINDOW (object);

	c2_application_window_remove (window->application, GTK_WINDOW (window));

	c2_mutex_destroy (&window->status_lock);
	c2_mutex_destroy (&window->progress_lock);

	if (GLADE_IS_XML (window->xml))
		gtk_object_unref (GTK_OBJECT (window->xml));
}

GtkWidget *
c2_window_new (C2Application *application, const gchar *title, gchar *type, gchar *icon)
{
	C2Window *window;

	window = gtk_type_new (c2_window_get_type ());

	c2_window_construct (window, application, title, type, icon);

	return GTK_WIDGET (window);
}

void
c2_window_construct (C2Window *window, C2Application *application, const gchar *title, gchar *type,
					gchar *icon)
{
	window->application = application;
	gtk_object_set_data (GTK_OBJECT (window), "type", type);

	gnome_app_construct (GNOME_APP (window), application->name, title ? title : "Cronos II");
	if (icon)
	{
		gtk_object_set_data (GTK_OBJECT (window), "icon", icon);
#ifdef USE_GNOME_WINDOW_ICON
		gnome_window_icon_set_from_file (GTK_WINDOW (window), icon);
#endif
	}

	gtk_signal_connect (GTK_OBJECT (window), "destroy",
						GTK_SIGNAL_FUNC (destroy), NULL);

	c2_application_window_add (application, GTK_WINDOW (window));
}

static gint
on_key_press_event (GtkWidget *widget, GdkEventKey *event)
{
	GtkWindow *window;
	GtkDirectionType direction = 0;
	gboolean handled;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_WINDOW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	window = GTK_WINDOW (widget);

	handled = FALSE;
  
	if (window->focus_widget &&
		window->focus_widget != widget &&
		GTK_WIDGET_IS_SENSITIVE (window->focus_widget))
	{
		handled = gtk_widget_event (window->focus_widget, (GdkEvent*) event);
	}
	
	if (!handled)
		handled = gtk_accel_groups_activate (GTK_OBJECT (window), event->keyval, event->state);
	
	if (!handled)
	{
		switch (event->keyval)
		{
			case GDK_space:
				if (window->focus_widget)
				{
					if (GTK_WIDGET_IS_SENSITIVE (window->focus_widget))
						gtk_widget_activate (window->focus_widget);
					handled = TRUE;
				}
				break;
			case GDK_Return:
			case GDK_KP_Enter:
				if (window->default_widget && GTK_WIDGET_IS_SENSITIVE (window->default_widget) &&
						(!window->focus_widget || !GTK_WIDGET_RECEIVES_DEFAULT (window->focus_widget)))
				{
					gtk_widget_activate (window->default_widget);
					handled = TRUE;
				}
				else if (window->focus_widget)
				{
					if (GTK_WIDGET_IS_SENSITIVE (window->focus_widget))
						gtk_widget_activate (window->focus_widget);
					handled = TRUE;
				}
				break;
			case GDK_Up:
			case GDK_Down:
			case GDK_Left:
			case GDK_Right:
			case GDK_KP_Up:
			case GDK_KP_Down:
			case GDK_KP_Left:
			case GDK_KP_Right:
			case GDK_Tab:
			case GDK_ISO_Left_Tab:
				switch (event->keyval)
				{
					case GDK_Up:
					case GDK_KP_Up:
						direction = GTK_DIR_UP;
						break;
					case GDK_Down:
					case GDK_KP_Down:
						direction = GTK_DIR_DOWN;
						break;
					case GDK_Left:
					case GDK_KP_Left:
						direction = GTK_DIR_LEFT;
						break;
					case GDK_Right:
					case GDK_KP_Right:
						direction = GTK_DIR_RIGHT;
						break;
					case GDK_Tab:
					case GDK_ISO_Left_Tab:
						if (event->state & GDK_SHIFT_MASK)
							direction = GTK_DIR_TAB_BACKWARD;
						else
							direction = GTK_DIR_TAB_FORWARD;
						break;
					default :
						direction = GTK_DIR_UP; /* never reached, but makes compiler happy */
				}
				
				gtk_container_focus (GTK_CONTAINER (widget), direction);
				
				if (!GTK_CONTAINER (window)->focus_child)
					gtk_window_set_focus (GTK_WINDOW (widget), NULL);
				else
					handled = TRUE;
				break;
		}
	}
	
	if (!handled && GTK_WIDGET_CLASS (parent_class)->key_press_event)
		handled = GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
	printf ("handled = %d\n", handled);
	
	return handled;
}

void
c2_window_set_contents (C2Window *window, GtkWidget *widget)
{
	gnome_app_set_contents (GNOME_APP (window), widget);
}

void
c2_window_set_contents_from_glade (C2Window *window, const gchar *dscp)
{
	GtkWidget *widget = glade_xml_get_widget (window->xml, dscp);

	if (widget)
		c2_window_set_contents (window, widget);
}

static gint16 report_timeout_id = 0;

void
c2_window_report (C2Window *window, C2WindowReportType type, const gchar *fmt, ...)
{
	va_list args;
	gchar *msg;

	va_start (args, fmt);
	msg = g_strdup_vprintf (fmt, args);
	va_end (args);

	if (!c2_mutex_trylock (&window->status_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");
		C2Pthread2 *data;

		if (!appbar)
		{
			g_free (msg);
			return;
		}

		data = g_new0 (C2Pthread2, 1);
		data->v1 = (gpointer) window;

		gnome_appbar_clear_stack (GNOME_APPBAR (appbar));
		gnome_appbar_push (GNOME_APPBAR (appbar), msg);
		data->v2 = (gpointer) gtk_timeout_add (3000, on_report_timeout, data);
		report_timeout_id = GPOINTER_TO_INT (data->v2);
		c2_mutex_unlock (&window->status_lock);
	} else
	{
		if (type == C2_WINDOW_REPORT_WARNING)
		{
			GtkWidget *dialog = gnome_warning_dialog (msg);

			c2_application_window_add (window->application, GTK_WINDOW (dialog));
			gtk_object_ref (GTK_OBJECT (dialog));

			gnome_dialog_run (GNOME_DIALOG (dialog));
			
			gtk_object_destroy (GTK_OBJECT (dialog));
		} else if (type == C2_WINDOW_REPORT_ERROR)
		{
			GtkWidget *dialog = gnome_error_dialog (msg);

			c2_application_window_add (window->application, GTK_WINDOW (dialog));
			gtk_object_ref (GTK_OBJECT (dialog));

			gnome_dialog_run (GNOME_DIALOG (dialog));

			gtk_object_destroy (GTK_OBJECT (dialog));
		}
	}

	g_free (msg);
}

static gint
on_report_timeout (gpointer data)
{
	C2Pthread2 *data2 = (C2Pthread2*) data;
	C2Window *window = C2_WINDOW (data2->v1);
	gint16 id = GPOINTER_TO_INT (data2->v2);

	if (C2_IS_WINDOW (window) && id == report_timeout_id && !c2_mutex_trylock (&window->status_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

		gnome_appbar_pop (GNOME_APPBAR (appbar));
		c2_mutex_unlock (&window->status_lock);
	}

	g_free (data2);
	
	return FALSE;
}

void
c2_window_set_activity (C2Window *window, gboolean state)
{
	if (state)
	{
		if (!c2_mutex_trylock (&window->progress_lock))
		{
			GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

			if (!appbar)
			{
				c2_mutex_unlock (&window->progress_lock);
				return;
			}

			gtk_progress_set_activity_mode (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), TRUE);
			gtk_timeout_add (10, on_activity_timeout, window);
		}
	} else
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

		gtk_progress_set_activity_mode (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), FALSE);
		gtk_progress_set_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), 0);
		c2_mutex_unlock (&window->progress_lock);
	}
}

static gint
on_activity_timeout (gpointer data)
{
	C2Window *window = C2_WINDOW (data);

	if (!c2_mutex_trylock (&window->progress_lock))
	{
		c2_mutex_unlock (&window->progress_lock);
	} else
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");
		static gboolean adding = TRUE;
		gfloat value = gtk_progress_get_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)));
	
		if (adding)
			value += 0.05;
		else
			value -= 0.05;
		
		if (adding && value > 1)
		{
			value -= 0.05;
			adding = !adding;
		} else if (!adding && value < 0)
		{
			value += 0.05;
			adding = !adding;
		}
		gtk_progress_set_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), value);
		return TRUE;
	}
	
	return FALSE;
}
