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
#ifndef __WIDGET_COMPOSER_H__
#define __WIDGET_COMPOSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <glade/glade.h>

#include <libcronosII/message.h>

#ifdef BUILDING_C2
#	include "widget-application.h"
#	include "widget-window.h"
#else
#	include <cronosII.h>
#endif

#define C2_COMPOSER(obj)					(GTK_CHECK_CAST (obj, c2_composer_get_type (), C2Composer))
#define C2_COMPOSER_CLASS(klass)			(GTK_CHECK_CLASS_CAST (klass, c2_composer_get_type (), C2ComposerClass))
#define C2_IS_COMPOSER(obj)					(GTK_CHECK_TYPE (obj, c2_composer_get_type ()))
#define C2_COMPOSER_CLASS_FW(obj)			(C2_COMPOSER_CLASS (((GtkObject*) (obj))->klass))

#define C2_COMPOSER_ACCOUNT					"Account:"
#define C2_COMPOSER_TO						"To:"
#define C2_COMPOSER_CC						"CC:"
#define C2_COMPOSER_BCC						"BCC:"
#define C2_COMPOSER_SUBJECT					"Subject:"
#define C2_COMPOSER_BODY					"Body:"

typedef struct _C2Composer C2Composer;
typedef struct _C2ComposerClass C2ComposerClass;
typedef struct _C2ComposerAttachment C2ComposerAttachment;
typedef enum _C2ComposerAction C2ComposerAction;
typedef enum _C2ComposerSendType C2ComposerSendType;
typedef enum _C2ComposerType C2ComposerType;

enum _C2ComposerType
{
	C2_COMPOSER_TYPE_INTERNAL,
	C2_COMPOSER_TYPE_EXTERNAL
};

enum _C2ComposerSendType
{
	C2_COMPOSER_SEND_NOW,
	C2_COMPOSER_SEND_LATER
};

enum _C2ComposerAction
{
	C2_COMPOSER_ACTION_NORMAL,
	C2_COMPOSER_ACTION_REPLY,
	C2_COMPOSER_ACTION_REPLY_ALL,
	C2_COMPOSER_ACTION_FORWARD,
	C2_COMPOSER_ACTION_DRAFT
};

struct _C2ComposerAttachment
{
	gchar *file;
	const gchar *type;
	gchar *description;
};

struct _C2Composer
{
	C2Window window;

	/* Editor */
	GtkWidget *editor;
	
	/* Type */
	C2ComposerType type;
	gchar *cmnd;

	/* Action */
	C2ComposerAction action;

	/* Save */
	gchar *file;

	/* Draft recovery */
	gint draft_id;
 
	/* Extra data */
	GList *eheaders;
};

struct _C2ComposerClass
{
	C2WindowClass parent_class;

	void (*changed_title) (C2Composer *composer, const gchar *title);

	void (*send_now) (C2Composer *composer);
	void (*send_later) (C2Composer *composer);
	
	void (*save_draft) (C2Composer *composer, gint draft_id);

	void (*add_attachment) (C2Composer *composer, gchar *file, gchar *description, gint nth);
};

/* GTK+ general functions */
GtkType
c2_composer_get_type						(void);

GtkWidget *
c2_composer_new								(C2Application *application);

void
c2_composer_construct						(C2Composer *composer, C2Application *application);

void
c2_composer_set_action						(C2Composer *composer, C2ComposerAction action);

/* Message handling */
C2Message *
c2_composer_get_message						(C2Composer *composer);

void
c2_composer_set_message						(C2Composer *composer, C2Message *message);

void
c2_composer_set_message_as_quote			(C2Composer *composer, C2Message *message);

/* Action handling */
void
c2_composer_save							(C2Composer *composer);

/* Data handling */
void
c2_composer_set_extra_field					(C2Composer *composer, const gchar *field,
											 const gchar *data);

void
c2_composer_set_contents_from_link			(C2Composer *composer, const gchar *link);

#ifdef __cplusplus
}
#endif

#endif
