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
#include "widget-network-traffic.h"

#define PIXELS_PER_SECOND	5
#define SECONDS_TO_DISPLAY	90
#define DRAWABLE_WIDTH		PIXELS_PER_SECOND * SECONDS_TO_DISPLAY
#define DRAWABLE_HEIGHT		300
#define TOP_MARGIN			10

static void
class_init									(C2NetworkTrafficClass *klass);

static void
init										(C2NetworkTraffic *nt);

static void
destroy										(GtkObject *obj);

static gint
configure_event								(GtkWidget *widget, GdkEventConfigure *e,
											 C2NetworkTraffic *nt);

static gint
expose_event								(GtkWidget *widget, GdkEventExpose *e,
											 C2NetworkTraffic *nt);

static gint
idle_timeout								(C2NetworkTraffic *nt);

static gint
timeout										(C2NetworkTraffic *nt);

static void
draw_background								(C2NetworkTraffic *nt);

static void
draw_top_speed								(C2NetworkTraffic *nt);

static void
draw_list									(C2NetworkTraffic *nt, GSList *list);

static void
draw_screen									(C2NetworkTraffic *nt);

static gfloat
get_pixels_per_byte							(C2NetworkTraffic *nt);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkDrawingAreaClass *parent_class = NULL;

GtkType
c2_network_traffic_get_type (void)
{
	static GtkType type = 0;
	
	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2NetworkTraffic",
			sizeof (C2NetworkTraffic),
			sizeof (C2NetworkTrafficClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gtk_drawing_area_get_type (), &info);
	}

	return type;
}

static void
class_init (C2NetworkTrafficClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gtk_drawing_area_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;
}

static void
init (C2NetworkTraffic *nt)
{
	nt->application = NULL;
	nt->pixmap = NULL;
	nt->top_speed = 1024;
	nt->recv = NULL;
	nt->send = NULL;
}

static void
destroy (GtkObject *obj)
{
	C2NetworkTraffic *nt;

	nt = C2_NETWORK_TRAFFIC (obj);
	gtk_timeout_remove (nt->timeout_id);
}

GtkWidget *
c2_network_traffic_new (C2Application *application)
{
	C2NetworkTraffic *nt;
	
	nt = gtk_type_new (c2_network_traffic_get_type ());

	gtk_widget_set_usize (GTK_WIDGET (nt), DRAWABLE_WIDTH+2, DRAWABLE_HEIGHT+2);
	gtk_drawing_area_size (GTK_DRAWING_AREA (nt),
							DRAWABLE_WIDTH+2, DRAWABLE_HEIGHT+2);
	gtk_signal_connect (GTK_OBJECT (nt), "expose_event",
						GTK_SIGNAL_FUNC (expose_event), nt);
	gtk_signal_connect (GTK_OBJECT (nt), "configure_event",
						GTK_SIGNAL_FUNC (configure_event), nt);

	nt->application = application;

	gtk_idle_add ((GtkFunction) idle_timeout, nt);
	nt->timeout_id = gtk_timeout_add (1000, (GtkFunction) timeout, nt);

	return GTK_WIDGET (nt);
}

static gint
configure_event (GtkWidget *widget, GdkEventConfigure *e, C2NetworkTraffic *nt)
{
	if (!nt->blue)
	{
		GdkColor blue = { 0, 0, 0, 0xffff };
		GdkColor red = { 0, 0xffff, 0, 0 };

		gdk_color_alloc (gdk_colormap_get_system (), &blue);
		gdk_color_alloc (gdk_colormap_get_system (), &red);

		nt->blue = gdk_gc_new (GTK_WIDGET (nt)->window);
		nt->red = gdk_gc_new (GTK_WIDGET (nt)->window);

		gdk_gc_set_foreground (nt->blue, &blue);
		gdk_gc_set_foreground (nt->red, &red);
	}
	
	if (nt->pixmap)
		gdk_pixmap_unref (nt->pixmap);

	nt->pixmap = gdk_pixmap_new (widget->window, widget->allocation.width,
							 widget->allocation.height, -1);
	
	return TRUE;
}

static gint
expose_event (GtkWidget *widget, GdkEventExpose *e, C2NetworkTraffic *nt)
{
	gdk_draw_pixmap (widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					 nt->pixmap, e->area.x, e->area.y, e->area.x, e->area.y,
					 e->area.width, e->area.height);
	
	return FALSE;
}

static gint
idle_timeout (C2NetworkTraffic *nt)
{
	timeout (nt);

	return FALSE;
}

static gint
timeout (C2NetworkTraffic *nt)
{
	gint current_recv, current_send, record;
	gint seconds_to_display; /* This is a dynamic value, the
							  * SECONDS_TO_DISPLAY constant
							  * is about how much we WISH to
							  * display, not how much we WILL
							  * display.
							  */
	GtkWidget *widget = GTK_WIDGET (nt);

	seconds_to_display = widget->allocation.width / PIXELS_PER_SECOND;

	current_recv = c2_net_speed_tracker_recv ();
	current_send = c2_net_speed_tracker_send ();

	if (g_slist_length (nt->recv) > seconds_to_display)
		nt->recv = g_slist_remove_link (nt->recv, g_slist_last (nt->recv));
	nt->recv = g_slist_prepend (nt->recv, (gpointer) current_recv);

	if (g_slist_length (nt->send) > seconds_to_display)
		nt->send = g_slist_remove_link (nt->send, g_slist_last (nt->send));
	nt->send = g_slist_prepend (nt->send, (gpointer) current_send);

	/* Update the maximum speed */
	record = current_send > current_recv ? current_send : current_recv;
	if (record > nt->top_speed)
		nt->top_speed = record;
	else
	{
		GSList *l;

		nt->top_speed = 1024;

		for (l = nt->send; l; l = g_slist_next (l))
			if (GPOINTER_TO_INT (l->data) > nt->top_speed)
				nt->top_speed = GPOINTER_TO_INT (l->data);
		for (l = nt->recv; l; l = g_slist_next (l))
			if (GPOINTER_TO_INT (l->data) > nt->top_speed)
				nt->top_speed = GPOINTER_TO_INT (l->data);
	}

	/* Time to draw */
	draw_background (nt);
	draw_top_speed (nt);
	draw_list (nt, nt->send);
	draw_list (nt, nt->recv);
	draw_screen (nt);
	
	return TRUE;
}

static void
draw_background (C2NetworkTraffic *nt)
{
	GtkWidget *widget = GTK_WIDGET (nt);
	
	gdk_draw_rectangle (nt->pixmap, widget->style->white_gc,
						TRUE, 0, 0, widget->allocation.width,
						widget->allocation.height);
	gdk_draw_rectangle (nt->pixmap, widget->style->black_gc,
						FALSE, 0, 0, widget->allocation.width-1,
						widget->allocation.height-1);
}

static void
draw_top_speed (C2NetworkTraffic *nt)
{
	gint x1, x2, y1, y2;
	gint text_width, text_height;
	gint top_speed_kb;
	gint i;
	gfloat pixels_per_byte;
	GdkFont *font;
	gchar *string;
	GtkWidget *widget = GTK_WIDGET (nt);

	/* Allocate the font */
	font = gdk_font_load ("-monotype-arial-bold-r-normal-*-*-110-*-*-p-*-iso8859-1");

	/* Allocate the value that are static */
	x1 = 1;

	/* Calculate which is the top_speed in Kb */
	top_speed_kb = nt->top_speed/1024;

	/* Calculate how many pixels should be drown per byte */
	pixels_per_byte = get_pixels_per_byte (nt);

	for (i = top_speed_kb; i > 0; i--)
	{
		string = g_strdup_printf ("(%d Kb/s)", i);
		
		if (i == top_speed_kb)
		{
			/* Calculate the dimensions of the text */
			text_height = gdk_string_height (font, string);
			text_width = gdk_string_width (font, string);
		} else
		{
			text_height = gdk_string_height (font, string);;
			text_width = 0;
		}

		/* Allocate the dynamic value that we calculated  */
		x2 = widget->allocation.width - text_width - 20;
		y1 = y2 = (text_height/2)+TOP_MARGIN + (pixels_per_byte * ((top_speed_kb-i)*1024));

		gdk_draw_line (nt->pixmap, widget->style->black_gc,
						x1, y1, x2, y2);

		if (i == top_speed_kb)
			gdk_draw_string (nt->pixmap, font, widget->style->black_gc,
							x2+10, y1+(text_height/2), string);

		g_free (string);
	}
}

static void
draw_list (C2NetworkTraffic *nt, GSList *list)
{
	GSList *l;
	gint x1, y1, x2, y2;
	gfloat pixels_per_byte;
	GtkWidget *widget = GTK_WIDGET (nt);
	gint width = widget->allocation.width - 2;
	gint height = widget->allocation.height - 2;
	GdkGC *color;

	pixels_per_byte = get_pixels_per_byte (nt);

	if (list == nt->recv)
		color = nt->blue;
	else
		color = nt->red;

	for (l = list, x2 = width; l; l = g_slist_next (l), x2 -= PIXELS_PER_SECOND)
	{
		gint bytes = GPOINTER_TO_INT (l->data);

		/* Calculate y2 */
		y2 = height - bytes * pixels_per_byte;
		if (!y2)
			y2++;

		/* Calculate y1 */
		if (l->next)
			y1 = height - (GPOINTER_TO_INT (l->next->data) * pixels_per_byte);
		else
			y1 = height;

		/* Calculate x1 */
		x1 = x2 - PIXELS_PER_SECOND;

		gdk_draw_line (nt->pixmap, color, x1, y1, x2, y2);

		if (l->next)
			x1 = (GPOINTER_TO_INT (l->next->data));
		else
			x1 = -7;
	}
}

static void
draw_screen (C2NetworkTraffic *nt)
{
	GdkRectangle rect;
	GtkWidget *widget = GTK_WIDGET (nt);

	rect.x = 0;
	rect.y = 0;
	rect.width = widget->allocation.width;
	rect.height = widget->allocation.height;
	
	gtk_widget_draw (widget, &rect);
}

static gfloat
get_pixels_per_byte (C2NetworkTraffic *nt)
{
	GtkWidget *widget = GTK_WIDGET (nt);
	
	return (gfloat) ((gint)widget->allocation.height - 2 - 1 - TOP_MARGIN) / ((gint)nt->top_speed);
}
