/* Cronos II - The GNOME mail client
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
#ifndef __WIDGET_DIALOG_PREFERENCES_H__
#define __WIDGET_DIALOG_PREFERENCES_H__

#ifdef __cplusplus
extern "C" {
#endif

#define C2_DIALOG_PREFERENCES(obj)			(GTK_CHECK_CAST (obj, c2_dialog_preferences_get_type (), C2DialogPreferences))
#define C2_DIALOG_PREFERENCES_CLASS(klass)	(GTK_CHECK_CLASS_CAST (klass, c2_dialog_preferences_get_type (), C2DialogPreferencesClass))
#define C2_IS_DIALOG_PREFENCES(obj)			(GTK_CHECK_TYPE (obj, c2_dialog_preferences_get_type ()))

typedef struct _C2DialogPreferences C2DialogPreferences;
typedef struct _C2DialogPreferencesClass C2DialogPreferencesClass;
typedef enum _C2DialogPreferencesSection C2DialogPreferencesSection;
typedef enum _C2DialogPreferencesSubSection C2DialogPreferencesSubSection;
typedef enum _C2DialogPreferencesKey C2DialogPreferencesKey;

#ifdef BUILDING_C2
#	include "widget-dialog.h"
#else
#	include <cronosII.h>
#endif

enum _C2DialogPreferencesKey
{
	/* [General] */
	/* Options */
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_START_CHECK,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_START_LOAD,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_EXIT_EXPUNGE,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_TIMEOUT_MARK,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_TIMEOUT_CHECK,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_INCOMING_WARN,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_OUTGOING_SENT_ITEMS,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_DELETE_USE_TRASH,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_DELETE_CONFIRMATION,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_OPTIONS_DELETE_ARCHIVE,
	
	/* Accounts */
	C2_DIALOG_PREFERENCES_KEY_GENERAL_ACCOUNTS,
	
	/* Paths */
	C2_DIALOG_PREFERENCES_KEY_GENERAL_PATHS_SAVE,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_PATHS_GET,
	C2_DIALOG_PREFERENCES_KEY_GENERAL_PATHS_SMART,

	/* Plugins */
	C2_DIALOG_PREFERENCES_KEY_GENERAL_PLUGINS,

	/* [Interface] */
	/* Fonts */
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_READED_MAILS,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILS,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_READED_MAILBOX,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_UNREADED_MAILBOX,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_COMPOSER_BODY,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_FONTS_MESSAGE_BODY,

	/* HTML */
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_HTML_IMAGES,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_HTML_IMAGES_CONFIRM,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_HTML_LINKS,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_HTML_LINKS_CONFIRM,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_HTML_INTERPRET_TEXT_PLAIN,

	/* Composer */
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_QUOTE_CHARACTER,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_QUOTE_COLOR,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_COMPOSER_EDITOR_CMND,

	/* Misc */
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_MISC_TITLE,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_MISC_DATE,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_MISC_ATTACHMENTS,
	C2_DIALOG_PREFERENCES_KEY_INTERFACE_MISC_MAIL_WARNING,

	/* [Advanced] */
	/* Misc */
	C2_DIALOG_PREFERENCES_KEY_ADVANCED_MISC_PROXY_HTTP,
	C2_DIALOG_PREFERENCES_KEY_ADVANCED_MISC_PROXY_HTTP_PORT,
	C2_DIALOG_PREFERENCES_KEY_ADVANCED_MISC_PROXY_FTP,
	C2_DIALOG_PREFERENCES_KEY_ADVANCED_MISC_PROXY_FTP_PORT
};

enum _C2DialogPreferencesSection
{
	C2_DIALOG_PREFERENCES_SECTION_GENERAL,
	C2_DIALOG_PREFERENCES_SECTION_INTERFACE,
	C2_DIALOG_PREFERENCES_SECTION_ADVANCED
};

enum _C2DialogPreferencesSubSection
{
	C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_OPTIONS = 1,
	C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_ACCOUNTS,
	C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_PATHS,
	C2_DIALOG_PREFERENCES_SUBSECTION_GENERAL_PLUGINS,
	
	C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_FONTS,
	C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_HTML,
	C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_COMPOSER,
	C2_DIALOG_PREFERENCES_SUBSECTION_INTERFACE_MISC,

	C2_DIALOG_PREFERENCES_SUBSECTION_ADVANCED_MISC
};

struct _C2DialogPreferences
{
	C2Dialog dialog;

	GtkWidget *interface_fonts_composer_body;
	GtkWidget *interface_fonts_message_body;
};

struct _C2DialogPreferencesClass
{
	C2DialogClass parent_class;

	void (*changed) (C2DialogPreferences *preferences, C2DialogPreferencesKey key, gpointer value);
};

GtkType
c2_dialog_preferences_get_type				(void);

GtkWidget *
c2_dialog_preferences_new					(C2Application *application);

void
c2_dialog_preferences_construct				(C2DialogPreferences *preferences, C2Application *application);

void
c2_dialog_preferences_set_section			(C2DialogPreferences *preferences,
											 C2DialogPreferencesSection section);

void
c2_dialog_preferences_set_subsection		(C2DialogPreferences *preferences,
											 C2DialogPreferencesSubSection subsection);

GtkWidget *
c2_dialog_preferences_account_editor_new	(C2Application *application, C2DialogPreferences *preferences,
											 C2Account *account);

#ifdef __cplusplus
}
#endif

#endif
