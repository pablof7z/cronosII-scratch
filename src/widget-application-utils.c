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
#include <libcronosII/error.h>
#include <libcronosII/request.h>

#include "preferences.h"
#include "main.h"
#include "widget-HTML.h"
#include "widget-application.h"
#include "widget-application-utils.h"
#include "widget-dialog-preferences.h"
#include "widget-mailbox-list.h"
#include "widget-network-traffic.h"
#include "widget-select-list.h"
#include "widget-window-main.h"

#define MAILBOX_TYPE_CRONOSII				"Cronos II"
#define MAILBOX_TYPE_IMAP					"IMAP"
#define MAILBOX_TYPE_SPOOL					_("Spool (local)")

/* TODO
 * 20011208 There's still a lack for the c2_application_dialog_select_file_get
 */

/* in widget-application.c */
extern void
on_mailbox_changed_mailboxes				(C2Mailbox *mailbox, C2Application *application);

gchar *
c2_application_str_number_to_string (gint number)
{
	switch (number)
	{
		case   0: return _("zero");
		case   1: return _("one");
		case   2: return _("two");
		case   3: return _("three");
		case   4: return _("four");
		case   5: return _("five");
		case   6: return _("six");
		case   7: return _("seven");
		case   8: return _("eight");
		case   9: return _("nine");
		case  10: return _("ten");
		case  11: return _("eleven");
		case  12: return _("twelve");
		case  13: return _("thirteen");
		case  14: return _("fourteen");
		case  15: return _("fifteen");
		case  16: return _("sixteen");
		case  17: return _("seventeen");
		case  18: return _("eighteen");
		case  19: return _("nineteen");
		case  20: return _("twenty");
	}

	return g_strdup_printf ("%d", number);
}

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
	GtkWidget *window, *widget;
	
	if (application->account)
		return TRUE;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_no_accounts");
	window = glade_xml_get_widget (xml, "dlg_no_accounts");

	if (gnome_preferences_get_dialog_centered ())
		gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	switch (gnome_dialog_run_and_close (GNOME_DIALOG (window)))
	{
		case 0:
			widget = c2_dialog_preferences_account_editor_new (application, NULL, NULL);
			gtk_widget_show (widget);
	}

	gtk_object_destroy (GTK_OBJECT (xml));

	return FALSE;
}

/**
 * c2_application_check_checkeable_account_exists
 * @application: C2Application where to act.
 *
 * This function will check if there is an account which
 * can be check (POP3) configured.
 *
 * Return Value:
 * %TRUE if there's an account checkeable or %FALSE.
 **/
gboolean
c2_application_check_checkeable_account_exists (C2Application *application)
{
	C2Account *account;

	for (account = application->account; C2_IS_ACCOUNT (account); account = account->next)
		if (c2_account_is_checkeable (account))
			return TRUE;

	return FALSE;
}

/**
 * c2_application_cut_text
 * @application: The C2Application object (just for compatibility, it won't be used..)
 * @font: GdkFont to be used.
 * @text: Text to be parsed.
 * @av_space: Available space.
 *
 * This function will check if the string @text, using the font @font,
 * fits in @av_space pixels, if not, it will cut the text (adding ...
 * at the end) where it does fits.
 *
 * Return Value:
 * The text that will fit the required space.
 **/
gchar *
c2_application_cut_text (C2Application *application, GdkFont *font, const gchar *text, guint16 av_space)
{
	gchar *buf, *buf2;
	guint16 tspace;
	gint i, len;

	c2_return_val_if_fail (text, NULL, C2EDATA);
	
	if (!av_space)
		return g_strdup ("");

	len = strlen (text);

	for (i = 0; i < len; i++)
	{
		if (!i)
			buf = g_strdup (text);
		else
		{
			buf2 = g_strndup (text, len-i);
			buf = g_strdup_printf ("%s...", buf2);
			g_free (buf2);
		}

		tspace = gdk_string_width (font, buf);

		if (tspace <= av_space)
			return buf;

		g_free (buf);
	}

	return g_strdup ("");
}

static void
on_dialog_missing_mailbox_inform_fix_clicked (GtkWidget *widget, GtkWidget *dialog)
{
	C2Application *application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (dialog), "application"));
	const gchar *name = (const gchar*) gtk_object_get_data (GTK_OBJECT (dialog), "name");
	C2Mailbox *mailbox;
	C2MailboxUseAs use_as = 0;
	gchar *path;
	gint id;
	GtkWidget *mdialog;

	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);

	if (c2_streq (name, C2_MAILBOX_INBOX))
		use_as = C2_MAILBOX_USE_AS_INBOX;
	else if (c2_streq (name, C2_MAILBOX_OUTBOX))
		use_as = C2_MAILBOX_USE_AS_OUTBOX;
	else if (c2_streq (name, C2_MAILBOX_SENT_ITEMS))
		use_as = C2_MAILBOX_USE_AS_SENT_ITEMS;
	else if (c2_streq (name, C2_MAILBOX_TRASH))
		use_as = C2_MAILBOX_USE_AS_TRASH;
	else if (c2_streq (name, C2_MAILBOX_DRAFTS))
		use_as = C2_MAILBOX_USE_AS_DRAFTS;
	
	if (!use_as || !C2_IS_MAILBOX ((mailbox = c2_mailbox_get_by_name (application->mailbox, name))))
	{
		mdialog = gnome_ok_dialog (_("Unable to automatically fix the problem, you'll have to do it manually "
								    "as explained before.\n"));
		gtk_widget_show (mdialog);
		return;
	}

	c2_mailbox_set_use_as (application->mailbox, mailbox, mailbox->use_as | use_as);

	if ((id = c2_application_get_mailbox_configuration_id_by_name (mailbox->name)) < 0)
	{
		mdialog = gnome_ok_dialog (_("Cronos II was able to fix the problem, but it was unable to "
								     "write the fix to the configuration, there must be something wrong "
									 "with your Gnome installation or with your Cronos II installation.\n"));
		gtk_widget_show (mdialog);
		return;
	}

	path = g_strdup_printf (PACKAGE "/Mailbox %d/use_as", id);
	gnome_config_set_int (path, mailbox->use_as);
	gnome_config_sync ();
	g_free (path);

	mdialog = gnome_ok_dialog (_("Cronos II fixed the problem successfully."));
	gtk_widget_show (mdialog);
}

static void
on_dialog_missing_mailbox_inform_close_clicked (GtkWidget *widget, GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
}

void
c2_application_dialog_missing_mailbox_inform (C2Application *application, const gchar *name)
{
	GtkWidget *dialog, *vbox, *wlabel, *ilabel;
	GtkStyle *style;
	gchar *text;

	dialog = c2_dialog_new (application, _("Warning"), "missing_mailbox", NULL,
							_("Automatically Fix"), GNOME_STOCK_BUTTON_CLOSE, NULL);
	vbox = GNOME_DIALOG (dialog)->vbox;
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
								GTK_SIGNAL_FUNC (on_dialog_missing_mailbox_inform_fix_clicked), dialog);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 1,
								GTK_SIGNAL_FUNC (on_dialog_missing_mailbox_inform_close_clicked), dialog);
	gtk_object_set_data (GTK_OBJECT (dialog), "application", application);
	gtk_object_set_data (GTK_OBJECT (dialog), "name", (gpointer) name);

	wlabel = gtk_label_new (_("Warning!"));
	style = gtk_style_copy (gtk_widget_get_style (wlabel));
	style->font = gdk_font_load (c2_font_bold);
	gtk_widget_set_style (wlabel, style);
	gtk_box_pack_start (GTK_BOX (vbox), wlabel, FALSE, TRUE, 0);
	gtk_widget_show (wlabel);

	text = g_strdup_printf (_("You have no mailbox marked to be used as «%s».\n"
							  "\n"
							  "Mark one of your mailboxes right clicking on it.\n"
							  "You can also create a new mailbox specifically for this usage.\n"
							  "\n"
							  "Cronos II won't be stable and might act weird if you don't do this.\n"
							  "\n"
							  "If you prefer Cronos II can automatically try to fix this problem.\n"),
							name);
	ilabel = gtk_label_new (text);
	gtk_misc_set_alignment (GTK_MISC (ilabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (ilabel), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (vbox), ilabel, FALSE, TRUE, 0);
	gtk_widget_show (ilabel);
	g_free (text);

	gtk_widget_show (dialog);
}

/**
 * c2_application_get_mailbox_configuration_id_by_name
 * @name: Name of searched mailbox.
 *
 * This function will return the Configuration ID
 * of the mailbox with name @name.
 * Mailboxes, in the c2 configuration file are
 * stored in sections, each mailbox is a separated
 * section, with an ID, in the form:
 * [Mailbox $configuration_id]
 * name=@name
 * id=0-0-1
 *
 * Return Value:
 * Configuration ID of mailbox or -1.
 **/
gint
c2_application_get_mailbox_configuration_id_by_name	(const gchar *name)
{
	gchar *prefix;
	gchar *gname;
	gint max = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=-1", NULL);
	gint i;
	
	c2_return_val_if_fail (name, -1, C2EDATA);

	for (i = 1; i <= max; i++)
	{
		prefix = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", i);
		gnome_config_push_prefix (prefix);

		gname = gnome_config_get_string ("name");

		if (c2_streq (gname, name))
		{
			g_free (gname);
			gnome_config_pop_prefix ();
			g_free (prefix);
			return i;
		}

		g_free (gname);
		gnome_config_pop_prefix ();
		g_free (prefix);
	}
	
	return -1;
}

static void
add_mailbox_dialog_type_selection_done (GtkWidget *widget, GladeXML *xml)
{
	GtkWidget *edata = glade_xml_get_widget (xml, "edata");
	GtkWidget *epathl = glade_xml_get_widget (xml, "epathl");
	GtkWidget *epath_imap = glade_xml_get_widget (xml, "epath_imap");
	GtkWidget *epath_spool = glade_xml_get_widget (xml, "epath_spool");
	GtkWidget *type = glade_xml_get_widget (xml, "type");
	gchar *selection;

	/* Get the selected type */
	if (GTK_BIN (type)->child)
	{
		GtkWidget *child = GTK_BIN (type)->child;

		if (GTK_LABEL (child))
		{
			gtk_label_get (GTK_LABEL (child), &selection);

			if (c2_streq (selection, MAILBOX_TYPE_CRONOSII))
				gtk_widget_hide (edata);
			else if (c2_streq (selection, MAILBOX_TYPE_IMAP))
			{
				gtk_widget_show (edata);
				gtk_widget_show (epathl);
				gtk_widget_show (epath_imap);
				gtk_widget_hide (epath_spool);
			} else if (c2_streq (selection, MAILBOX_TYPE_SPOOL))
			{
				gtk_widget_hide (epath_imap);
				gtk_widget_show (edata);
				gtk_widget_show (epathl);
				gtk_widget_show (epath_spool);
			}
		}
	}
}

void
c2_application_dialog_add_mailbox (C2Application *application)
{
	GtkWidget *dialog;
	GtkWidget *menuitem, *wbuf;
	GtkWidget *menu;
	GtkWidget *mlist;
	GtkWidget *scroll;
	GtkWidget *wmain;
	GtkOptionMenu *option_menu;
	GladeXML *xml;
	gchar *get_path;

	/* Create the dialog */
	dialog = c2_dialog_new (application, _("New mailbox"), "new_mailbox",
							NULL,
							GNOME_STOCK_BUTTON_HELP,
							GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_mailbox_properties_contents");
	C2_DIALOG (dialog)->xml = xml;

	/* Pack the contents */
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), glade_xml_get_widget (xml,
							"dlg_mailbox_properties_contents"), TRUE, TRUE, 0);

	/* Create the Type menu */
	menu = gtk_menu_new ();
	option_menu = GTK_OPTION_MENU (glade_xml_get_widget (xml, "type"));
	gtk_signal_connect (GTK_OBJECT (menu), "selection_done",
						GTK_SIGNAL_FUNC (add_mailbox_dialog_type_selection_done), xml);
	
	menuitem = gtk_menu_item_new_with_label (MAILBOX_TYPE_CRONOSII);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (MAILBOX_TYPE_SPOOL);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (MAILBOX_TYPE_IMAP);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	gtk_option_menu_set_menu (option_menu, menu);
	gtk_option_menu_set_history (option_menu, 0);

	/* Create the mailbox list widget */
	mlist = c2_mailbox_list_new (application);
	scroll = glade_xml_get_widget (xml, "scroll");
	gtk_container_add (GTK_CONTAINER (scroll), mlist);
	gtk_widget_show (mlist);

	/* Try to get a window main and check which is the mailbox selected there */
	if (C2_IS_WINDOW_MAIN ((wmain = c2_application_window_get (application, "wmain"))))
	{
		GtkObject *selected_object;

		selected_object = c2_window_main_get_mlist_selection (C2_WINDOW_MAIN (wmain));

		c2_mailbox_list_set_selected_object (C2_MAILBOX_LIST (mlist), selected_object);
	}

	/* Get the focus for the name widget */
	gtk_widget_grab_focus (glade_xml_get_widget (xml, "name"));

	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	gtk_object_set_data (GTK_OBJECT (application), "add_mailbox_dialog::xml", xml);

	c2_preferences_get_general_paths_get (get_path);
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (glade_xml_get_widget (xml, "epath_spool")),
									get_path);
	g_free (get_path);

re_run_add_mailbox_dialog:
	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 1:
			{
				C2MailboxType type = 0;
				gchar *name, *path = NULL;
				C2Mailbox *parent, *mailbox = NULL;
				gint config_id;
				gchar *query;
				const gchar *egg = NULL;

				name = gtk_entry_get_text (GTK_ENTRY (glade_xml_get_widget (xml, "name")));

				/* [TODO] Hehe, a little eastern egg :) */
				if (c2_streq (name, "!GET FUNNY!"))
				{
					gnome_config_set_bool (PACKAGE "/EastEgg/funny", TRUE);
					egg = ".:[FunnY]:.";
				} else if (c2_streq (name, "MARIANA TE AMO"))
				{
					gnome_config_set_bool (PACKAGE "/EastEgg/mariana", TRUE);
					egg = "Mariana";
				} else if (c2_streq (name, "COGITO, ERGO SUM"))
				{
					gnome_config_set_bool (PACKAGE "/EastEgg/cogito", TRUE);
					egg = "Cogito";
				} else if (c2_streq (name, "Nietzsche :: ALSO SPRACH ZARATHRUSTA"))
				{
					gnome_config_set_bool (PACKAGE "/EastEgg/nietzche", TRUE);
					egg = _("Nietzche was wrong");
				}

				if (egg)
				{
					GtkWidget *e_dialog;
					gchar *e_str;
					
					gnome_config_sync ();
					e_str = g_strdup_printf (_("Congratulations, dude! You found the \"%s\" eastern egg of Cronos II!"), egg);
					e_dialog = gnome_ok_dialog_parented (e_str, GTK_WINDOW (dialog));

					gnome_dialog_run_and_close (GNOME_DIALOG (e_dialog));

					gtk_widget_destroy (dialog);

					g_free (e_str);
					return;
				}

				/* Check if the name is valid */
				if (!name || !strlen (name) ||
					c2_mailbox_get_by_name (application->mailbox, name))
				{
					GladeXML *err_xml;
					GtkWidget *err_dialog;
					
					err_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_mailbox_err");
					err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_err");

					gtk_window_set_modal (GTK_WINDOW (err_dialog), TRUE);
					gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));

					gtk_object_destroy (GTK_OBJECT (err_xml));

					goto re_run_add_mailbox_dialog;
				}

				/* Check if the data is enough for the type */
				wbuf = glade_xml_get_widget (xml, "type");

				if (GTK_BIN (wbuf)->child)
				{
					GtkWidget *child = GTK_BIN (wbuf)->child;
					gchar *query;
					
					if (GTK_LABEL (child))
					{
						gtk_label_get (GTK_LABEL (child), &query);
						
						if (c2_streq (query, MAILBOX_TYPE_CRONOSII))
							type = C2_MAILBOX_CRONOSII;
						else if (c2_streq (query, MAILBOX_TYPE_IMAP))
						{
							type = C2_MAILBOX_IMAP;
							path = gtk_entry_get_text (GTK_ENTRY (
													glade_xml_get_widget (xml, "epath_imap")));

						} else if (c2_streq (query, MAILBOX_TYPE_SPOOL))
						{
							type = C2_MAILBOX_SPOOL;
							path = gtk_entry_get_text (GTK_ENTRY (
													gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (
													glade_xml_get_widget (xml, "epath_spool")))));

							if (!strlen (path))
							{
								GladeXML *err_xml;
								GtkWidget *err_dialog;

								err_xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"),
															"dlg_mailbox_not_enough_data");
								err_dialog = glade_xml_get_widget (err_xml, "dlg_mailbox_not_enough_data");
								
								gtk_window_set_modal (GTK_WINDOW (err_dialog), TRUE);
								gnome_dialog_run_and_close (GNOME_DIALOG (err_dialog));
								
								gtk_object_destroy (GTK_OBJECT (err_xml));
								
								goto re_run_add_mailbox_dialog;
							}
						}
					}
				}

				/* Get parent mailbox */
				parent = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (mlist));

				switch (type)
				{
					case C2_MAILBOX_CRONOSII:
						mailbox = c2_mailbox_new_with_parent (
											&C2_WINDOW (wmain)->application->mailbox,
											name, parent ? parent->id : NULL, type,
											C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
						break;
					case C2_MAILBOX_IMAP:
						{
							C2IMAP *imap;
							
							imap = parent->protocol.IMAP.imap;
							
							mailbox = c2_mailbox_new_with_parent (
												&C2_WINDOW (wmain)->application->mailbox,
												name, parent ? parent->id : NULL, type,
												C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING,
												imap, TRUE);
						}
						break;
					case C2_MAILBOX_SPOOL:
						mailbox = c2_mailbox_new_with_parent (
											&C2_WINDOW (wmain)->application->mailbox,
											name, parent ? parent->id : NULL, type,
											C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING, path);
						break;
				}

				if (!mailbox)
				{
					c2_window_report (C2_WINDOW (wmain), C2_WINDOW_REPORT_WARNING,
										error_list[C2_FAIL_MAILBOX_CREATE], name);
					switch (type)
					{
						case C2_MAILBOX_IMAP:
						case C2_MAILBOX_SPOOL:
							g_free (path);
						case C2_MAILBOX_CRONOSII:
							g_free (name);
					}

					return;
				}

				if (c2_preferences_get_general_options_start_load ())
					c2_mailbox_load_db (mailbox);

				/* If this is the first mailbox we need
				 * to connect the application to the
				 * signal changed_mailboxes and we
				 * also have to reemit the signal,
				 * so the application knows about it.
				 */
				if (!parent)
				{
					gtk_signal_connect (GTK_OBJECT (mailbox), "changed_mailboxes",
									GTK_SIGNAL_FUNC (on_mailbox_changed_mailboxes), application);
					gtk_signal_emit_by_name (GTK_OBJECT (mailbox), "changed_mailboxes");
				}
				
				config_id = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
				query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
				gnome_config_push_prefix (query);
				
				gnome_config_set_string ("name", mailbox->name);
				gnome_config_set_string ("id", mailbox->id);
				gnome_config_set_int ("type", mailbox->type);
				gnome_config_set_int ("sort_by", mailbox->sort_by);
				gnome_config_set_int ("sort_type", mailbox->sort_type);
				
				switch (mailbox->type)
				{
					case C2_MAILBOX_SPOOL:
						gnome_config_set_string ("path", mailbox->protocol.spool.path);
						break;
				}
				gnome_config_pop_prefix ();
				g_free (query);
				
				gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);
				gnome_config_sync ();
			}
		case 2:
			gtk_window_set_modal (GTK_WINDOW (dialog), FALSE);
			gtk_object_destroy (GTK_OBJECT (dialog));
			break;
		case 0:
			/* [TODO]
			 * c2_application_help_show (wmain->application, "c2help://add_mailbox_dialog");
			 */
			break;
	}		
}

static void
dialog_remove_mailbox_thread (C2Dialog *dialog)
{
	C2Application *application;
	C2Mailbox *mailbox;
	GtkWidget *button;
	gint toggle = 0;

	application = dialog->application;
	mailbox = C2_MAILBOX (gtk_object_get_data (GTK_OBJECT (dialog), "mailbox"));
	
	switch (mailbox->type)
	{
		case C2_MAILBOX_IMAP:
		case C2_MAILBOX_SPOOL:
			button = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dialog), "toggle"));
			toggle = GTK_TOGGLE_BUTTON (button)->active;
	}

	c2_mailbox_remove (&application->mailbox, mailbox);

	switch (mailbox->type)
	{
		case C2_MAILBOX_IMAP:
			break;
			
		case C2_MAILBOX_SPOOL:
			if (!toggle)
				unlink (mailbox->protocol.spool.path);
			break;
			
		case C2_MAILBOX_CRONOSII:
		case C2_MAILBOX_OTHER:
			break;
	}

	gtk_object_unref (GTK_OBJECT (mailbox));
}

void
c2_application_dialog_remove_mailbox (C2Application *application, C2Mailbox *mailbox)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *button = NULL;
	pthread_t thread;
	
	c2_return_if_fail (C2_IS_APPLICATION (application), C2EDATA);
	c2_return_if_fail (C2_IS_MAILBOX (mailbox), C2EDATA);

	dialog = c2_dialog_new (application, _("Mailbox deletion confirm"),
									"remove_mailbox", NULL, GNOME_STOCK_BUTTON_OK,
									GNOME_STOCK_BUTTON_CANCEL, NULL);
	gtk_object_set_data (GTK_OBJECT (dialog), "mailbox", mailbox);

	label = gtk_label_new (_("You are about to delete a mailbox.\n"
							 "After deleting it you will not be able to\n"
							 "recover your messages."));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), label, TRUE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	
	/* According to the type of mailbox is the dialog we have to display */
	switch (mailbox->type)
	{
		case C2_MAILBOX_CRONOSII:
		case C2_MAILBOX_OTHER:
			button = NULL;
			break;
			
		case C2_MAILBOX_IMAP:
			button = gtk_check_button_new_with_label (_("Just unsubscribe, do not delete the mailbox."));
			break;
			
		case C2_MAILBOX_SPOOL:
			button = gtk_check_button_new_with_label (_("Do not delete the spool file."));
			break;
	}

	if (button)
	{
		gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), button, FALSE, TRUE, 0);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
		gtk_widget_show (button);
		gtk_object_set_data (GTK_OBJECT (dialog), "toggle", button);
	}

	switch (gnome_dialog_run (GNOME_DIALOG (dialog)))
	{
		case 0:
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (dialog_remove_mailbox_thread), dialog);
			break;

		default:
		case 1:
			gnome_dialog_close (GNOME_DIALOG (dialog));
	}
}

static gint
on_dialog_incoming_mail_warning_darea_expose_event (GtkWidget *widget, GdkEventExpose *e, GtkWidget *window)
{
	GdkPixmap *pixmap;
	gchar *str;
	gint text_pos_x, text_pos_y, mails;
	GtkWidget *darea;
	GdkFont *font;

	mails = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "mails"));
	text_pos_x = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "text_pos_x"));
	text_pos_y = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "text_pos_y"));
	font = (GdkFont*) gtk_object_get_data (GTK_OBJECT (window), "font");
	
	pixmap = gtk_object_get_data (GTK_OBJECT (window), "pixmap");
	
	gdk_draw_pixmap (widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  pixmap,
		  e->area.x, e->area.y,
		  e->area.x, e->area.y,
		  e->area.width, e->area.height);
	
	darea = GTK_BIN (window)->child;
	
	if (mails > 1)
		str = g_strdup_printf (_("You've got %d new mails!"), mails);
	else
		str = g_strdup (_("You've got a new mail!"));
	gdk_draw_string (widget->window, font,
				darea->style->black_gc,
				text_pos_x-(gdk_string_width (font, str)/2), text_pos_y, str);
	g_free (str);

	return TRUE;
}

static gint
on_dialog_incoming_mail_warning_darea_configure_event (GtkWidget *widget, GdkEventConfigure *e, GtkWidget *window)
{
	GtkWidget *gpixmap;
	GdkPixmap *pixmap;
	gint mails;
	gint text_pos_x, text_pos_y;
	gint width, height;

	pixmap = gtk_object_get_data (GTK_OBJECT (window), "pixmap");
	mails = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "mails"));
	text_pos_x = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "text_pos_x"));
	text_pos_y = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "text_pos_y"));
	width = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "width"));
	height = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "height"));

	if (pixmap)
		gdk_pixmap_unref (pixmap);
	
	gpixmap = gnome_pixmap_new_from_file_at_size (PKGDATADIR "/pixmaps/mail_warn.png", width, height);
	pixmap = GNOME_PIXMAP (gpixmap)->pixmap;
	gtk_object_set_data (GTK_OBJECT (window), "pixmap", pixmap);
/*
	gdk_draw_pixmap (widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  pixmap,
		  0, 0,
		  width, height,
		  width, height);*/
	
/*	if (mails > 1)
	{
		str = g_strdup_printf (_("You've got %d new mails!"), mails);
		gdk_draw_string (pixmap, gdk_font_load (c2_font_bold),
				widget->style->white_gc,
				text_pos_x, text_pos_y, str);
	}*/

	return TRUE;
}

static gint
on_dialog_incoming_mail_warning_timeout (GtkWidget *window)
{
	gchar *pos;
	gint width, height, x, y;
	gint retval = TRUE;
	gboolean showing;
	C2Application *application;

	printf ("Ejecutando %s\n", __PRETTY_FUNCTION__);

	application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (window), "application"));
	pos = gtk_object_get_data (GTK_OBJECT (window), "pos");
	width = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "width"));
	height = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "height"));
	y = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "y"));

	if (strstr (pos, "LEFT"))
		x = 0;
	else
		x = gdk_screen_width () - width;

	if ((showing = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "showing"))))
		y+=6;
	else
		y-=6;
	
	if (strstr (pos, "TOP"))
	{
		gtk_window_reposition (GTK_WINDOW (window), x, -height+y);

		if (y >= height || y <= 0)
			retval = FALSE;
	} else
	{
		gtk_window_reposition (GTK_WINDOW (window), x, gdk_screen_height ()-y);

		if (y >= height || y <= 0)
			retval = FALSE;
	}

	gtk_object_set_data (GTK_OBJECT (window), "y", (gpointer) y);

	if (!showing && y <= 0)
		gtk_widget_destroy (window);

	if (!retval)
		c2_application_dialog_incoming_mail_warning (application);

	printf ("Terminando de ejecutar %s\n", __PRETTY_FUNCTION__);

	return retval;
}

static gint
on_dialog_incoming_mail_warning_pixmap_button_press_event (GtkWidget *widget, GdkEventButton *e, GtkWidget *window)
{
	C2Application *application;
	
	switch (e->button)
	{
		default:
			application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (window), "application"));

			c2_application_command (application, C2_COMMAND_WINDOW_MAIN_RAISE);
		case 1:
			gtk_object_remove_data (GTK_OBJECT (window), "showing");
			gtk_timeout_add (20, (GtkFunction) on_dialog_incoming_mail_warning_timeout, window);
	}

	return TRUE;
}

void
c2_application_dialog_incoming_mail_warning (C2Application *application)
{
	GtkWidget *window;
	GtkWidget *darea;
	GtkWidget *gpixmap;
	gchar *pos;
	gint height = 210, width = 360;
	gint text_pos_x = width/2, text_pos_y = height-10;
	GtkTooltips *tooltips;
	GdkFont *font;

	if (GTK_IS_WINDOW ((window = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (application), "mail_warn_window"))))
	{
		gint mails;
		GdkEventExpose *e;
		
		darea = GTK_BIN (window)->child;

		mails = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (window), "mails"));
		mails++;
		gtk_object_set_data (GTK_OBJECT (window), "mails", (gpointer) mails);

		e = g_new0 (GdkEventExpose, 1);

		e->area.x = 0;
		e->area.y = 0;
		e->area.width = width;
		e->area.height = height;
		gtk_signal_emit_by_name (GTK_OBJECT (darea), "expose_event", e);
	} else
	{
		window = gtk_window_new (GTK_WINDOW_POPUP);
		gpixmap = gnome_pixmap_new_from_file_at_size (PKGDATADIR "/pixmaps/mail_warn.png", width, height);
		darea = gtk_drawing_area_new ();
		gtk_drawing_area_size (GTK_DRAWING_AREA (darea), width, height);
		gtk_container_add (GTK_CONTAINER (window), darea);
		tooltips = gtk_tooltips_new ();
		gtk_tooltips_set_tip (tooltips, window,
								_("Click with left button: Hide this warning.\n"
								  "Click with other button: Open Cronos II."), NULL);
		font = gdk_font_load (c2_font_bold);
		
		pos = c2_preferences_get_interface_misc_mail_warning ();

		gtk_object_set_data (GTK_OBJECT (window), "application", application);
		gtk_object_set_data (GTK_OBJECT (window), "pos", (gpointer) pos);
		gtk_object_set_data (GTK_OBJECT (window), "width", (gpointer) width);
		gtk_object_set_data (GTK_OBJECT (window), "height", (gpointer) height);
		gtk_object_set_data (GTK_OBJECT (window), "pixmap", GNOME_PIXMAP (gpixmap)->pixmap);
		gtk_object_set_data (GTK_OBJECT (window), "text_pos_x", (gpointer) text_pos_x);
		gtk_object_set_data (GTK_OBJECT (window), "text_pos_y", (gpointer) text_pos_y);
		gtk_object_set_data (GTK_OBJECT (window), "font", (gpointer) font);
		gtk_object_set_data (GTK_OBJECT (window), "mails", (gpointer) 1);
		gtk_object_set_data (GTK_OBJECT (window), "showing", (gpointer) 1);

		gtk_signal_connect (GTK_OBJECT (darea), "expose_event",
				GTK_SIGNAL_FUNC (on_dialog_incoming_mail_warning_darea_expose_event), window);
		gtk_signal_connect (GTK_OBJECT (darea), "configure_event",
				GTK_SIGNAL_FUNC (on_dialog_incoming_mail_warning_darea_configure_event), window);
		gtk_signal_connect (GTK_OBJECT (darea), "button_press_event",
				GTK_SIGNAL_FUNC (on_dialog_incoming_mail_warning_pixmap_button_press_event), window);
		gtk_widget_set_events (darea, GDK_EXPOSURE_MASK
						 | GDK_LEAVE_NOTIFY_MASK
						 | GDK_BUTTON_PRESS_MASK
						 | GDK_POINTER_MOTION_MASK
						 | GDK_POINTER_MOTION_HINT_MASK);
		gtk_widget_show (darea);
		gtk_widget_show (gpixmap);

		gtk_object_set_data (GTK_OBJECT (application), "mail_warn_window", window);
		
		if (c2_streq (pos, "TOP LEFT"))
			gtk_widget_popup (window, 0, -height);
		else if (c2_streq (pos, "TOP RIGHT"))
			gtk_widget_popup (window, gdk_screen_width () - width, -height);
		else if (c2_streq (pos, "BOTTOM LEFT"))
			gtk_widget_popup (window, 0, gdk_screen_height () + height);
		else
			gtk_widget_popup (window, gdk_screen_width () - width,
									gdk_screen_height () + height);

		gtk_timeout_add (20, (GtkFunction) on_dialog_incoming_mail_warning_timeout, window);
	}
}

static void
on_dialog_add_features_thread_disconnect (C2Request *request, C2NetObjectByte *byte, gboolean success,
											GladeXML *xml)
{
	GtkWidget *widget;
	const gchar *source, *ptr;

	if (!success)
	{
		gchar *buf;
		
		gdk_threads_enter ();
		buf = g_strdup_printf (_("Failed: %s"), c2_error_object_get (GTK_OBJECT (request)));
		widget = glade_xml_get_widget (xml, "status_label");
		gtk_label_set_text (GTK_LABEL (widget), buf);
		g_free (buf);
		gdk_threads_leave ();

		return;
	}

	source = c2_request_get_source (request);

	widget = glade_xml_get_widget (xml, "select_list");

	gdk_threads_enter ();
	gtk_widget_hide (glade_xml_get_widget (xml, "status_label"));
	gdk_threads_leave ();

	for (ptr = source; *ptr != '\0';)
	{
		gchar *name, *size, *desc, *url;
		gint s, len;

		name = c2_str_get_word (0, ptr, '\r');
		size = c2_str_get_word (1, ptr, '\r');
		desc = c2_str_get_word (2, ptr, '\r');
		url  = c2_str_get_word (3, ptr, '\r');

		len = strlen (size);
		
		s = atoi (size);
		g_free (size);
		if ((s/1024) > 1024)
			size = g_strdup_printf ("%.2f Mb", (gfloat) (s/1024)/1024);
		else
			size = g_strdup_printf ("%.2f Kb", (gfloat) (s/1024));

		ptr += strlen (name) + len + strlen (desc) + strlen (url) + 4;

		if (*ptr != '\0')
			ptr++;

		{
			gchar *row[] = { name, size, desc };
			
			gdk_threads_enter ();
			c2_select_list_append_item (C2_SELECT_LIST (widget), row, g_strndup (url, strlen (url)-1));
			gdk_threads_leave ();
		}

		g_free (name);
		g_free (size);
		g_free (desc);
		g_free (url);
	}	

	gtk_object_destroy (GTK_OBJECT (request));
}

static void
dialog_add_features_thread (GladeXML *xml)
{
	C2Request *request;

	request = c2_request_new (FEATURES_URL);
	gtk_signal_connect (GTK_OBJECT (request), "disconnect",
						GTK_SIGNAL_FUNC (on_dialog_add_features_thread_disconnect), xml);
	c2_request_run (request);
}

void
c2_application_dialog_add_features (C2Application *application)
{
	GtkWidget *widget;
	GladeXML *xml;
	pthread_t thread;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_add_features");

	pthread_create (&thread, NULL, C2_PTHREAD_FUNC (dialog_add_features_thread), xml);

	widget = glade_xml_get_widget (xml, "dlg_add_features");
	c2_application_window_add (application, GTK_WINDOW (widget));

	switch (gnome_dialog_run (GNOME_DIALOG (widget)))
	{
		case 0:
			break;
		case 1:
			break;
		default:
		case 2:
			c2_application_window_remove (application, (GTK_WINDOW (widget)));
			gnome_dialog_close (GNOME_DIALOG (widget));
			break;
	}
	
	gtk_object_destroy (GTK_OBJECT (xml));
}

static void
on_dialog_network_traffic_close_clicked (GtkWidget *widget, GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
}

void
c2_application_dialog_network_traffic (C2Application *application)
{
	GtkWidget *dialog;
	GtkWidget *nt;

	dialog = c2_dialog_new (application, _("Network Traffic"), "network-traffic",
							PKGDATADIR "/pixmaps/send-receive16.png",
							GNOME_STOCK_BUTTON_CLOSE, NULL);
	gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
								on_dialog_network_traffic_close_clicked, dialog);
	
	nt = c2_network_traffic_new (application);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), nt, TRUE, TRUE, 0);
	gtk_widget_show (nt);

	gtk_widget_show (dialog);
}

static void
on_dialog_release_information_show_at_start_btn_toggled (GtkWidget *button)
{
	c2_preferences_set_extra_release_information_show (GTK_TOGGLE_BUTTON (button)->active);
	c2_preferences_commit ();
}

static gboolean
on_dialog_release_information_window_delete_event (GtkWidget *widget, GdkEvent *e, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

static void
on_dialog_release_information_close_clicked (GtkWidget *widget, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	GtkWidget *window = glade_xml_get_widget (xml, "dlg_release_information");
	
	c2_application_window_remove (application, GTK_WINDOW (window));
	gtk_object_destroy (GTK_OBJECT (window));
	gtk_object_destroy (GTK_OBJECT (xml));
}

/**
 * c2_application_dialog_release_information
 *
 * This function will show a dialog with information
 * about the current release.
 **/
void
c2_application_dialog_release_information (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget, *html, *parent;
	gint i;
	gchar *buf;
#ifdef USE_GTKHTML
	GtkWidget *scroll;
#endif
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_release_information");
	gtk_object_set_data (GTK_OBJECT (xml), "application", application);

	for (i = 0; i < 3; i++)
	{
		gchar *id, *file;

		switch (i)
		{
			case 0:
				id = "welcome_box";
				file = PKGDATADIR "/welcome.html";
				break;
			case 1:
				id = "bugs_box";
				file = PKGDATADIR "/bugs.html";
				break;
			case 2:
				id = "features_box";
				file = PKGDATADIR "/features.html";
				break;
			default:
				id = file = NULL;
		}
		
		widget = glade_xml_get_widget (xml, id);
		
#ifdef USE_GTKHTML
		scroll = gtk_viewport_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (widget), scroll, TRUE, TRUE, 0);
		gtk_widget_show (scroll);
		gtk_viewport_set_shadow_type (GTK_VIEWPORT (scroll), GTK_SHADOW_IN);
	
		parent = gtk_scrolled_window_new (NULL, NULL);
		gtk_container_add (GTK_CONTAINER (scroll), parent);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#elif defined (USE_GTKXMHTML)
		parent = gtk_viewport_new (NULL, NULL);
		gtk_box_pack_start (GTK_BOX (widget), parent, TRUE, TRUE, 0);
#else
		parent = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (widget), parent, TRUE, TRUE, 0);
#endif
		gtk_widget_show (parent);

		html = c2_html_new (application);
		gtk_container_add (GTK_CONTAINER (parent), html);
		gtk_widget_show (html);

		/* Load the file */
		if (c2_get_file (file, &buf) > 0)
			c2_html_set_content_from_string (C2_HTML (html), buf);
//		g_free (buf);
	}

	widget = glade_xml_get_widget (xml, "show_at_start_btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
								c2_preferences_get_extra_release_information_show ());
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_dialog_release_information_show_at_start_btn_toggled), NULL);
	
	widget = glade_xml_get_widget (xml, "dlg_release_information");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_release_information_window_delete_event), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_release_information_close_clicked), xml);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gtk_widget_show (widget);
}

void
c2_application_dialog_default_mailer_check (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget;
	gint i;
	gchar *buf;
	gboolean elm_is_set = TRUE;

	/* TODO */
	return;

#if 0
	/* Checks:
	 *   o "cronosII -l" must be the handler for "mailto:" URLs
	 *   o "cronosII -f" must be the handler for .elm files.
	 */
	if (!(buf = gnome_mime_type ("message.elm")))
		elm_is_set = FALSE;

	if (elm_is_set)
	{
		C2_DEBUG (buf);
		buf = gnome_mime_program (buf);

		if (!strstr (buf, "cronosII -f"))
			elm_is_set = FALSE;
		C2_DEBUG (buf);
	}

	if (!elm_is_set)
	{
		
	}
		

	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_default_mailer");
	gtk_object_set_data (GTK_OBJECT (xml), "application", application);

	widget = glade_xml_get_widget (xml, "check_again_btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
								c2_preferences_get_extra_default_mailer_check ());
	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
						GTK_SIGNAL_FUNC (on_dialog_default_mailer_check_check_again_btn_toggled), NULL);
	
	widget = glade_xml_get_widget (xml, "dlg_default_mailer");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_default_mailer_check_window_delete_event), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_default_mailer_check_yes_clicked), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 1,
						GTK_SIGNAL_FUNC (on_dialog_default_mailer_check_no_clicked), xml);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gtk_widget_show (widget);
#endif
}



static gboolean
on_dialog_about_window_delete_event (GtkWidget *widget, GdkEvent *e)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (widget), "application"));
	c2_application_window_remove (application, GTK_WINDOW (widget));

	return TRUE;
}

static void
on_dialog_about_close_clicked (GtkWidget *widget, GtkWidget *window)
{
	C2Application *application = C2_APPLICATION (gtk_object_get_data (GTK_OBJECT (window), "application"));
	c2_application_window_remove (application, GTK_WINDOW (window));
	
	gtk_object_destroy (GTK_OBJECT (window));
}

void
c2_application_dialog_about (C2Application *application)
{
	GtkWidget *widget, *button;
	const gchar *authors[] =
	{
		"The Cronos II Hackers Team <cronosII-hackers@lists.sf.net>:",
		"Pablo Fernández <sandokan@cronosII.org>",
		"Bosko Blagojevic <falling@cronosII.org>",
		"Peter Gossner <petegozz@cronosII.org>",
		"Daniel Fairhead <daniel@cronosII.org>",
		NULL
	};

	widget = gnome_about_new (_("Cronos II"), " - Mariana -",
							_("© Copyright 2000-2002 Pablo Fernández"),
							authors,
							_("Cronos II is a powerful and light mail client, designed for the Gnome Desktop.\n"
							  "It will administer your e-mails, let you communicate with your friends, "
							  "partners, couple and it will let you receive all that spam and all those "
							  "mails you did never ask for! Oh, yes, it will save you from the wolf "
							  "in case you are going to your grandmom's house in the middle of the woods"
							  " (although this last feature hasn't been tested that much since there're no "
							  "many volunteers to test it...)"),
							PKGDATADIR "/pixmaps/splash.png");
	gtk_object_set_data (GTK_OBJECT (widget), "application", application);

	button = gnome_href_new (URL, _("Visit the Cronos II website for the latest news!"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (widget)->vbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);
	
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_about_window_delete_event), NULL);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_about_close_clicked), widget);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gnome_dialog_run_and_close (GNOME_DIALOG (widget));
}

static gboolean
on_dialog_getting_in_touch_window_delete_event (GtkWidget *widget, GdkEvent *e, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (xml));

	return TRUE;
}

static void
on_dialog_getting_in_touch_close_clicked (GtkWidget *widget, GladeXML *xml)
{
	C2Application *application = C2_APPLICATION (
							gtk_object_get_data (GTK_OBJECT (xml), "application"));
	GtkWidget *window = glade_xml_get_widget (xml, "dlg_getting_in_touch");
	
	c2_application_window_remove (application, GTK_WINDOW (window));
	gtk_object_destroy (GTK_OBJECT (window));
	gtk_object_destroy (GTK_OBJECT (xml));
}

void
c2_application_dialog_getting_in_touch (C2Application *application)
{
	GladeXML *xml;
	GtkWidget *widget, *html, *parent;
	gchar *buf;
#ifdef USE_GTKHTML
	GtkWidget *scroll;
#endif
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_getting_in_touch");

	widget = glade_xml_get_widget (xml, "contents_box");
	gtk_object_set_data (GTK_OBJECT (xml), "application", application);
		
#ifdef USE_GTKHTML
	scroll = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (widget), scroll, TRUE, TRUE, 0);
	gtk_widget_show (scroll);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (scroll), GTK_SHADOW_IN);
	
	parent = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scroll), parent);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#elif defined (USE_GTKXMHTML)
	parent = gtk_viewport_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (widget), parent, TRUE, TRUE, 0);
#else
	parent = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (widget), parent, TRUE, TRUE, 0);
#endif
	gtk_widget_show (parent);

	html = c2_html_new (application);
	gtk_container_add (GTK_CONTAINER (parent), html);
	gtk_widget_show (html);

	/* Load the file */
	if (c2_get_file (PKGDATADIR G_DIR_SEPARATOR_S "touch.html", &buf) > 0)
		c2_html_set_content_from_string (C2_HTML (html), buf);
	g_free (buf);

	widget = glade_xml_get_widget (xml, "dlg_getting_in_touch");
	gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
						GTK_SIGNAL_FUNC (on_dialog_getting_in_touch_window_delete_event), xml);
	gnome_dialog_button_connect (GNOME_DIALOG (widget), 0,
						GTK_SIGNAL_FUNC (on_dialog_getting_in_touch_close_clicked), xml);
	c2_application_window_add (application, GTK_WINDOW (widget));

	gtk_widget_show (widget);
}

/**
 * c2_application_dialog_send_unsent_mails
 * @application: Application object.
 *
 * This function will popup a dialog asking the
 * user if s/he wants to send the unsent mails
 * in the «Outbox» mailbox.
 *
 * Return Value:
 * %TRUE if mails should be sent or %FALSE.
 **/
gboolean
c2_application_dialog_send_unsent_mails (C2Application *application)
{
	GtkWidget *widget;
	C2Mailbox *outbox;
	gint mails;
	gchar *label;
	gint reply;

	outbox = c2_mailbox_get_by_usage (application->mailbox, C2_MAILBOX_USE_AS_OUTBOX);
	mails = c2_db_length (outbox);
	if (!mails)
		return FALSE;

	if (mails > 1)
		label = g_strdup_printf (_("There are %d unsent mails.\n"
								   "\n"
								   "Do you want to send them now?"), mails);
	else
		label = g_strdup (_("There is an unsent message.\n"
							"\n"
							"Do you want to send it now?"));
	
	widget = gnome_question_dialog_modal (label, NULL, NULL);
	c2_application_window_add (application, GTK_WINDOW (widget));
	gnome_dialog_close_hides (GNOME_DIALOG (widget), TRUE);
	
	reply = gnome_dialog_run (GNOME_DIALOG (widget));
	
	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_widget_destroy (widget);
	g_free (label);
	
	return !reply;
}

C2Mailbox *
c2_application_dialog_select_mailbox (C2Application *application, GtkWindow *parent)
{
	GladeXML *xml;
	C2Mailbox *mailbox = NULL;
	GtkWidget *widget;
	GtkWidget *scroll;
	GtkWidget *mlist;

	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_select_mailbox");
	
	scroll = glade_xml_get_widget (xml, "scroll");
	mlist = c2_mailbox_list_new (application);
	gtk_container_add (GTK_CONTAINER (scroll), mlist);
	gtk_widget_show (mlist);

	widget = glade_xml_get_widget (xml, "dlg_select_mailbox");

	c2_application_window_add (application, GTK_WINDOW (widget));
	gtk_object_ref (GTK_OBJECT (widget));

	if (GTK_IS_WINDOW (parent))
		gtk_window_set_transient_for (GTK_WINDOW (widget), parent);

	switch (gnome_dialog_run (GNOME_DIALOG (widget)))
	{
		case 0:
			mailbox = c2_mailbox_list_get_selected_mailbox (C2_MAILBOX_LIST (mlist));
			break;
		case 1:
			mailbox = NULL;
			break;
	}

	c2_application_window_remove (application, GTK_WINDOW (widget));
	gtk_object_destroy (GTK_OBJECT (widget));
	gtk_object_unref (GTK_OBJECT (xml));

	return mailbox;
}

static void
on_application_dialog_select_file_save_ok_clicked (GtkWidget *widget, gint *button)
{
	*button = 1;
	gtk_main_quit ();
}

static void
on_application_dialog_select_file_save_cancel_clicked (GtkWidget *widget, gint *button)
{
	*button = 0;
	gtk_main_quit ();
}

static void
on_application_dialog_select_file_save_delete_event (GtkWidget *widget, GdkEvent *e, gint *button)
{
	*button = 0;
	gtk_main_quit ();
}

FILE *
c2_application_dialog_select_file_save (C2Application *application, gchar **file)
{
	GtkWidget *filesel;
	gint button;
	gchar *buf;
	gchar *dir;
	FILE *fd;

	filesel = gtk_file_selection_new (_("Save message"));
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	
	c2_application_window_add (application, GTK_WINDOW (filesel));

	c2_preferences_get_general_paths_save (dir);
	buf = g_strdup_printf ("%s/%s", dir, *file);
	gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), buf);
	g_free (buf);
	g_free (dir);
	
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button), "clicked",
						GTK_SIGNAL_FUNC (on_application_dialog_select_file_save_ok_clicked), &button);
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->cancel_button), "clicked",
						GTK_SIGNAL_FUNC (on_application_dialog_select_file_save_cancel_clicked), &button);
	gtk_signal_connect (GTK_OBJECT (filesel), "delete_event",
						GTK_SIGNAL_FUNC (on_application_dialog_select_file_save_delete_event), &button);

	gtk_widget_show (filesel);
rerun:
	gtk_main ();

	if (!button)
	{
		c2_error_set (C2USRCNCL);
		c2_application_window_remove (application, GTK_WINDOW (filesel));
		gtk_widget_destroy (filesel);

		return NULL;
	}

	buf = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));

	/* Check if a directory was selected */
	if (c2_file_is_directory (buf))
	{
		GladeXML *xml;
		GtkWidget *dialog;
		gint ret;
		
		xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_directory_selected");
		
		dialog = glade_xml_get_widget (xml, "dlg_directory_selected");

		ret = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

		gtk_object_destroy (GTK_OBJECT (xml));

		if (ret == 1)
			goto rerun;
	}

	/* Check that the file doesn't exist yet */
	if (!c2_file_exists (buf))
	{
		GladeXML *xml;
		GtkWidget *dialog;
		gint ret;
		
		xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_confirm_overwrite");
		
		dialog = glade_xml_get_widget (xml, "dlg_confirm_overwrite");

		ret = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

		gtk_object_destroy (GTK_OBJECT (xml));

		if (ret == 1)
			goto rerun;
	}

	gtk_widget_hide (filesel);
	if (file)
	{
		g_free (*file);
		*file = g_strdup (buf);
	}

	if (!(fd = fopen (buf, "w")))
		c2_error_set (-errno);

	if (c2_preferences_get_general_paths_smart ())
	{
		gchar *dir, *buf2;

		buf2 = g_dirname (buf);
		dir = g_strdup_printf ("%s" G_DIR_SEPARATOR_S, buf2);
		g_free (buf2);
		c2_preferences_set_general_paths_save (dir);
		c2_preferences_commit ();
		g_free (dir);
	}
	c2_application_window_remove (application, GTK_WINDOW (filesel));
	gtk_widget_destroy (filesel);

	return fd;
}

/**
 * c2_application_dialog_mail_source
 * @application: Application object.
 * @message: Message to display.
 *
 * This function will popup a dialog which will show
 * the source code of @message.
 **/
void
c2_application_dialog_mail_source (C2Application *application, C2Message *message)
{
	GladeXML *xml;
	GtkWidget *widget;
	GdkColor green = { 0, 0x2d00, 0x8a00, 0x5700 };
	GdkColor red = { 0, 0xa500, 0x2900, 0x2900 };
	GdkFont *font_normal, *font_bold;
	gchar *ptr, *line = NULL;
	GtkText *text;

	gdk_color_alloc (gdk_colormap_get_system (), &green);
	gdk_color_alloc (gdk_colormap_get_system (), &red);
	font_normal = gdk_font_load ("-adobe-courier-medium-r-normal-*-*-140-*-*-m-*-iso8859-1");
	font_bold = gdk_font_load ("-adobe-courier-bold-r-normal-*-*-140-*-*-m-*-iso8859-1");
	
	xml = glade_xml_new (C2_APPLICATION_GLADE_FILE ("dialogs"), "dlg_mail_source");

	/* Set the header */
	widget = glade_xml_get_widget (xml, "text");
	text = GTK_TEXT (widget);
	
	for (ptr = message->header; *ptr != '\0';)
	{
		/* Get the current line */
		line = c2_str_get_line (ptr);
		ptr += strlen (line);

		if (*line == ' ' || *line == '\t')
			gtk_text_insert (text, font_normal, NULL, NULL, line, -1);
		else
		{
			gchar *word = c2_str_get_word (0, line, ' ');
			gint length = strlen (word);
			
			gtk_text_insert (text,
							*(word+length-1) == ':' ? font_bold : font_normal,
							c2_strneq (word, "X-", 2) ? &red : &green,
							NULL, word, length);
			gtk_text_insert (text, font_normal, NULL, NULL, line+length, -1);
			g_free (word);
		}
		g_free (line);
	}

	/* Set the body */
	gtk_text_insert (text, font_normal, NULL, NULL, "\n\n", 2);
	gtk_text_insert (text, font_normal, NULL, NULL, message->body, -1);
	
	widget = glade_xml_get_widget (xml, "dlg_mail_source");
	gtk_widget_set_usize (widget, c2_preferences_get_window_main_width (),
								  c2_preferences_get_window_main_height ());
	gnome_dialog_run_and_close (GNOME_DIALOG (widget));
	gtk_object_destroy (GTK_OBJECT (xml));
}

void
c2_application_new_message_alert (C2Application *application)
{
	GtkWidget *window;
	
	window = gtk_window_new (GTK_WINDOW_POPUP);
	
}
