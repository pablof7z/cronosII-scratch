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
#include "widget-application.h"

/**
 * c2_application_check_account_exists
 * @application: C2Application where to act.
 * 
 * This function checks if theres an account configured.
 *
 * Return Value:
 * %TRUE if there's an account, %FALSE if not.
 **/
gboolean
c2_application_check_account_exists (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *window;
	
	if (application->account)
		return TRUE;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("cronosII"), "dlg_no_accounts");
	window = glade_xml_get_widget (xml, "dlg_no_accounts");

	switch (gnome_dialog_run_and_close (GNOME_DIALOG (window)))
	{
		case 0:
			c2_preferences_new ();
	}

	return FALSE;
}
