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
#ifndef __WIDGET_APPLICATION_UTILS_H__
#define __WIDGET_APPLICATION_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libcronosII/message.h>

gboolean
c2_application_check_account_exists			(C2Application *application);

gboolean
c2_application_check_checkeable_account_exists (C2Application *application);

gchar *
c2_application_cut_text						(C2Application *application, GdkFont *font,
											 const gchar *text, guint16 av_space);

/* Dialogs */
void
c2_application_dialog_missing_mailbox_inform	(C2Application *application, const gchar *name);

void
c2_application_dialog_add_mailbox			(C2Application *application);

void
c2_application_dialog_incoming_mail_warning	(C2Application *application);

void
c2_application_dialog_add_features			(C2Application *application);

void
c2_application_dialog_network_traffic		(C2Application *application);

void
c2_application_dialog_release_information	(C2Application *application);

void
c2_application_dialog_about					(C2Application *application);

void
c2_application_dialog_getting_in_touch		(C2Application *application);

gboolean
c2_application_dialog_send_unsent_mails		(C2Application *application);

C2Mailbox *
c2_application_dialog_select_mailbox		(C2Application *application, GtkWindow *parent);

FILE *
c2_application_dialog_select_file_save		(C2Application *application, gchar **file);

void
c2_application_dialog_mail_source			(C2Application *application, C2Message *message);

#ifdef __cplusplus
}
#endif

#endif
