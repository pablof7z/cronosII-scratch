/*  Cronos II - The GNOME Mail Client
 *  Copyright (C) 2000-2001 Pablo Fernández López
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
#ifndef __WIDGET_HTML_GTKXMHTML_H__
#define __WIDGET_HTML_GTKXMHTML_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_C2
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#ifdef USE_GTKXMHTML

#include <gtk-xmhtml/gtk-xmhtml.h>

XmImageInfo *
c2_html_gtkxmhtml_image_load				(GtkWidget *widget, const gchar *href);

void
c2_html_gtkxmhtml_anchor_track				(GtkWidget *widget, XmHTMLAnchorCallbackStruct *cbs,
											 GtkWidget *appbar);

#endif

#ifdef __cplusplus
}
#endif

#endif
