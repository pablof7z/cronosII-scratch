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
#include "widget-dialog-network-traffic.h"

#define PIXELS_PER_SECOND	5
#define SECONDS_TO_DISPLAY	90
#define DRAWABLE_WIDTH		PIXELS_PER_SECOND * SECONDS_TO_DISPLAY
#define DRAWABLE_HEIGHT		300
#define TOP_MARGIN			10

static void
class_init									(C2DialogNetworkTrafficClass *klass);

static void
init										(C2DialogNetworkTraffic *nt);

static gint
configure_event								(GtkWidget *widget, GdkEventConfigure *e,
											 C2DialogNetworkTraffic *nt);

static gint
expose_event								(GtkWidget *widget, GdkEventExpose *e,
											 C2DialogNetworkTraffic *nt);

static gint
timeout										(C2DialogNetworkTraffic *nt);

static void
draw_background								(C2DialogNetworkTraffic *nt);

static void
draw_top_speed								(C2DialogNetworkTraffic *nt);

static void
draw_list									(C2DialogNetworkTraffic *nt, GSList *list);

static void
draw_screen									(C2DialogNetworkTraffic *nt);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2DialogClass *parent_class = NULL;

GtkType
c2_dialog_network_traffic_get_type (void)
{
	static GtkType type = 0;
	
	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2DialogNetworkTraffic",
			sizeof (C2DialogNetworkTraffic),
			sizeof (C2DialogNetworkTrafficClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_dialog_get_type (), &info);
	}

	return type;
}

static void
class_init (C2DialogNetworkTrafficClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (c2_dialog_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
init (C2DialogNetworkTraffic *nt)
{
	nt->pixmap = NULL;
	nt->top_speed = 1024;
	nt->recv = NULL;
	nt->send = NULL;
}

GtkWidget *
c2_dialog_network_traffic_new (C2Application *application)
{
	C2DialogNetworkTraffic *nt;
	const gchar *buttons [] = { GNOME_STOCK_BUTTON_CLOSE, NULL };	
	
	nt = gtk_type_new (c2_dialog_network_traffic_get_type ());

	c2_dialog_construct (C2_DIALOG (nt), application, _("Network Traffic"), "network-traffic",
						 "send-receive16.png", buttons);

	nt->darea = gtk_drawing_area_new ();
	gtk_drawing_area_size (GTK_DRAWING_AREA (nt->darea),
							DRAWABLE_WIDTH+2, DRAWABLE_HEIGHT+2);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (nt)->vbox), nt->darea, TRUE, TRUE, 0);
	gtk_widget_show (nt->darea);
	gtk_signal_connect (GTK_OBJECT (nt->darea), "expose_event",
						GTK_SIGNAL_FUNC (expose_event), nt);
	gtk_signal_connect (GTK_OBJECT (nt->darea), "configure_event",
						GTK_SIGNAL_FUNC (configure_event), nt);	

	timeout (nt);
	gtk_timeout_add (1000, timeout, nt);

	return GTK_WIDGET (nt);
}

static gint
configure_event (GtkWidget *widget, GdkEventConfigure *e, C2DialogNetworkTraffic *nt)
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
expose_event (GtkWidget *widget, GdkEventExpose *e, C2DialogNetworkTraffic *nt)
{
	gdk_draw_pixmap (widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					 nt->pixmap, e->area.x, e->area.y, e->area.x, e->area.y,
					 e->area.width, e->area.height);
	
	return FALSE;
}

static gint
timeout (C2DialogNetworkTraffic *nt)
{
	gint current_recv, current_send;

	current_recv = c2_net_speed_tracker_recv ();
	current_send = c2_net_speed_tracker_send ();

	if (g_slist_length (nt->recv) > SECONDS_TO_DISPLAY)
		nt->recv = g_slist_remove_link (nt->recv, g_slist_last (nt->recv));
	nt->recv = g_slist_prepend (nt->recv, (gpointer) current_recv);

	if (g_slist_length (nt->send) > SECONDS_TO_DISPLAY)
		nt->send = g_slist_remove_link (nt->send, g_slist_last (nt->send));
	nt->send = g_slist_prepend (nt->send, (gpointer) current_send);

	/* Update the maximum speed */
	if (current_send > nt->top_speed)
		nt->top_speed = current_send;
	if (current_recv > nt->top_speed)
		nt->top_speed = current_recv;

	/* Time to draw */
	draw_background (nt);
	draw_top_speed (nt);
	draw_list (nt, nt->recv);
	draw_list (nt, nt->send);
	draw_screen (nt);
	
	return TRUE;
}

static void
draw_background (C2DialogNetworkTraffic *nt)
{
	gdk_draw_rectangle (nt->pixmap, nt->darea->style->white_gc,
						TRUE, 0, 0, nt->darea->allocation.width,
						nt->darea->allocation.height);
	gdk_draw_rectangle (nt->pixmap, nt->darea->style->black_gc,
						FALSE, 0, 0, nt->darea->allocation.width-1,
						nt->darea->allocation.height-1);
}

static void
draw_top_speed (C2DialogNetworkTraffic *nt)
{
	gint x1, x2, y1, y2;
	gint text_width, text_height;
	gint top_speed_kb;
	GdkFont *font;
	gchar *string;

	/* Allocate the value that are static */
	x1 = 1;

	/* Calculate which is the top_speed in Kb */
	top_speed_kb = nt->top_speed/1024;
	if (!top_speed_kb)
		top_speed_kb = 1;
	string = g_strdup_printf ("(%d Kb/s)", top_speed_kb);

	/* Allocate the font */
	font = gdk_font_load ("-monotype-arial-bold-r-normal-*-*-110-*-*-p-*-iso8859-1");

	/* Calculate the dimensions of the text */
	text_height = gdk_string_height (font, string);
	text_width = gdk_string_width (font, string);

	/* Allocate the dynamic value that we calculated  */
	x2 = DRAWABLE_WIDTH - text_width - 20;
	y1 = y2 = (text_height/2)+TOP_MARGIN;

	gdk_draw_line (nt->pixmap, nt->darea->style->black_gc,
					x1, y1, x2, y2);
	gdk_draw_string (nt->pixmap, font, nt->darea->style->black_gc,
					x2+10, y1+(text_height/2), string);
}

static void
draw_list (C2DialogNetworkTraffic *nt, GSList *list)
{
	GSList *l;
	gint x1, y1, x2, y2;
	gdouble pixels_per_byte;
	gint width = nt->darea->allocation.width - 2;
	gint height = nt->darea->allocation.height - 2;
	GdkGC *color;

	if (list == nt->recv)
		color = &nt->blue;
	else
		color = &nt->red;

	pixels_per_byte = (gfloat) ((gint)height - 1 - TOP_MARGIN) / ((gint)nt->top_speed);

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

		gdk_draw_line (nt->pixmap, nt->blue, x1, y1, x2, y2);

		if (l->next)
			x1 = (GPOINTER_TO_INT (l->next->data));
		else
			x1 = -7;
		printf ("%f: y1 %d (%d) y2 %d (%d)\n", pixels_per_byte, y1, x1, y2, bytes);
	}
}

static void
draw_screen (C2DialogNetworkTraffic *nt)
{
	GdkRectangle rect;

	rect.x = 0;
	rect.y = 0;
	rect.width = nt->darea->allocation.width;
	rect.height = nt->darea->allocation.height;
	
	gtk_widget_draw (nt->darea, &rect);
}
