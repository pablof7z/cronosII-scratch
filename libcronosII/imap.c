/*  Cronos II - The GNOME mail client
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
/*
 * Maintainer(s) of this file:
 * 		* Bosko Blagojevic
 * Code of this file by:
 * 		* Bosko Blagojevic
 */
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "mailbox.h"
#include "net-object.h"
#include "utils.h"
#include "utils-date.h"
#include "account.h"
#include "imap.h"
#include "i18n.h" 

/* C2 IMAP Module in the process of being engineered by Bosko */
/* TODO: Set marks for emails */
/* TODO: Re-Implement recursive folder listing (its too slow) */
/* TODO: Function that handles untagged messages that come unwarranted */
/* (in progress) TODO: Elegent network problem handling (reconnecting, etc) */
/* (done!) TODO: Implement subscribtion facility */
/* (done!) TODO: Get messages */
/* (done!) TODO: Add messages */
/* (done!) TODO: Delete messages */
/* (done!) TODO: Get list of messages */
/* (done!) TODO: Load a mailbox */
/* (done!) TODO: Login (at least plain-text for now) */
/* (done!) TODO: Create a test module */
/* (done!) TODO: Internal folder managment + syncronization */
/* (done!) TODO: Create Folders */
/* (done!) TODO: Delete Folders */
/* (done!) TODO: Rename Folders */
/* (done!) TODO: Function for reading server replies */
/* (done!) TODO: Get list of folders */

#define NET_READ_FAILED  _("Internal socket read operation failed, connection is most likely broken")
#define NET_WRITE_FAILED _("Internal socket write operation failed, connection is most likely broken")

const gint C2TagLen = 14; /* strlen("CronosII-XXXX "); */

/* Private GtkObject functions */
static void
class_init									(C2IMAPClass *klass);

static void
init										(C2IMAP *imap);

static void
destroy										(GtkObject *object);

/* Private IMAP functions */
static void
c2_imap_on_net_traffic (gpointer *data, gint source, GdkInputCondition condition);

static C2IMAPPending *
c2_imap_new_pending (C2IMAP *imap, tag_t tag);

static void
c2_imap_remove_pending (C2IMAP *imap, tag_t tag);

static void
c2_imap_unlock_pending (C2IMAP *imap);

static gchar *
c2_imap_get_server_reply (C2IMAP *imap, tag_t tag);

static gint
c2_imap_plaintext_login (C2IMAP *imap);

static gchar*
c2_imap_get_mailbox_list(C2IMAP *imap, const gchar *reference, const gchar *name);

static gint
c2_imap_select_mailbox (C2IMAP *imap, C2Mailbox *mailbox);

static gint
c2_imap_close_mailbox (C2IMAP *imap);

/* Internal Mailbox Managment */

static gint
c2_imap_mailbox_loop(C2IMAP *imap, C2Mailbox *head);

static guint
c2_imap_get_mailbox_level (gchar *name);

static gchar *
c2_imap_get_mailbox_hierarchy (gchar *name, guint level);

static gchar *
c2_imap_get_full_mailbox_name (C2IMAP *imap, C2Mailbox *mailbox);

/* Misc. functions */
static tag_t
c2_imap_get_tag								(C2IMAP *imap);

static void
c2_imap_set_error(C2IMAP *imap, const gchar *error);

static gint
c2_imap_reconnect(C2IMAP *imap);

enum 
{
	LOGIN,
	LOGIN_FAILED,
	MAILBOX_LIST,
	INCOMING_MAIL,
	LOGOUT,
	NET_ERROR,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObjectClass *parent_class = NULL;

GtkType
c2_imap_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2IMAP",
			sizeof (C2IMAP),
			sizeof (C2IMAPClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_net_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2IMAPClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass*) klass;

	parent_class = gtk_type_class(c2_net_object_get_type() );
	
	signals[LOGIN] =
		gtk_signal_new ("login",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, login),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[LOGIN_FAILED] =
		gtk_signal_new ("login_failed",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, login_failed),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[MAILBOX_LIST] =
		gtk_signal_new ("mailbox_list",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, mailbox_list),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[INCOMING_MAIL] =
		gtk_signal_new ("incoming_mail",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, incoming_mail),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[LOGOUT] =
		gtk_signal_new ("logout",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, logout),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[NET_ERROR] =
		gtk_signal_new("net_error",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, net_error),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->login = NULL;
	klass->login_failed = NULL;
	klass->mailbox_list = NULL;
	klass->incoming_mail = NULL;
	klass->logout = NULL;
	klass->net_error = NULL;
	
	object_class->destroy = destroy;
}

static void
init (C2IMAP *imap)
{
	imap->account = NULL;
	imap->auth = 0;
	imap->cmnd = 1;
	imap->mailboxes = NULL;
	imap->selected_mailbox = NULL;
	imap->user = NULL;
	imap->pass = NULL;
	imap->path = NULL;
	imap->hash = NULL;
	imap->pending = NULL;
	imap->login = NULL;
	imap->auth_remember = TRUE;
	imap->only_subscribed = TRUE;
	imap->state = C2IMAPDisconnected;
	c2_mutex_init(&imap->lock);
}

C2IMAP *
c2_imap_new (C2Account *account, const gchar *host, const guint port, const gchar *user, 
		const gchar *pass, const gchar *path, const C2IMAPAuthenticationType auth, const gboolean ssl)
{
	C2IMAP *imap;

	imap = gtk_type_new (C2_TYPE_IMAP);
	imap->account = account;
	
	imap->user = g_strdup(user);
	imap->pass = g_strdup(pass);
	imap->path = g_strdup(path);
	
	switch(auth)
	{
		case C2_IMAP_AUTHENTICATION_PLAINTEXT:
			imap->login = c2_imap_plaintext_login;
			break;
	}

	c2_net_object_construct (C2_NET_OBJECT (imap), host, port, ssl);	
	return imap;
}

/**
 * c2_imap_init
 * @imap: IMAP object.
 *
 * This function will start an IMAP object,
 * it will connect the object, make it login, etc.
 * 
 * Return Value:
 * 0 on success, -1 on failure.
 **/
gint
c2_imap_init (C2IMAP *imap)
{
	C2NetObjectByte *byte;
	
	if(imap->mailboxes && !c2_net_object_is_offline(C2_NET_OBJECT(imap)))
		return 0;
	
	c2_mutex_lock(&imap->lock);
	if(!(byte = c2_net_object_run(C2_NET_OBJECT(imap))))
	{
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		c2_imap_set_error(imap, _("Error connecting to IMAP server."));
		c2_mutex_unlock(&imap->lock);
		return -1;
	}

	gdk_input_add(byte->sock, GDK_INPUT_READ, (GdkInputFunction)c2_imap_on_net_traffic, imap);
	imap->state = C2IMAPNonAuthenticated;
	
	if(imap->login(imap) < 0)
	{
		gtk_signal_emit(GTK_OBJECT(imap), signals[LOGIN_FAILED]);
		c2_mutex_unlock(&imap->lock);
		return -1;
	}
	
	imap->state = C2IMAPAuthenticated;
	g_print ("%s (%s@%s)\n", __PRETTY_FUNCTION__, imap->user, C2_NET_OBJECT (imap)->host);
	
	c2_mutex_unlock(&imap->lock);
	return 0;
}

static void
destroy(GtkObject *object)
{
	C2IMAP *imap = C2_IMAP(object);
	GList *ptr;
	GSList *ptr2;
	
	g_free(imap->user);
	g_free(imap->pass);
	g_free(imap->path);
	
	for(ptr = imap->hash; ptr; ptr = ptr->next)
		g_free(ptr->data);
	g_list_free(imap->hash);
	
	for(ptr2 = imap->pending; ptr2; ptr2 = ptr2->next)
		g_free(ptr2->data);
	g_slist_free(imap->pending);
	
	c2_mutex_destroy(&imap->lock);
}

/** c2_imap_reconnect
 * 
 * @imap: A locked imap object
 *
 * This function attempts to reconnect and restore 
 * proper state of an imap object.
 * 
 * Return Value:
 * 0 on success, -1 otherwise
 **/
static gint
c2_imap_reconnect(C2IMAP *imap)
{
	c2_net_object_disconnect(C2_NET_OBJECT(imap));
	c2_mutex_unlock(&imap->lock);
	if(c2_imap_init < 0)
	{
		c2_mutex_lock(&imap->lock);
		return -1;
	}
	c2_mutex_lock(&imap->lock);
	
	if(imap->state == C2IMAPSelected)
		if(c2_imap_select_mailbox(imap, imap->selected_mailbox) < 0)
	  {
			c2_net_object_disconnect(C2_NET_OBJECT(imap));
			return -1;
		}

	return 0;
}

static void
c2_imap_on_net_traffic (gpointer *data, gint source, GdkInputCondition condition)
{
	C2IMAP *imap = C2_IMAP(data);
	gchar *buf, *buf2;
	gchar *final = NULL;
	tag_t tag = 0;
	
	c2_mutex_lock(&imap->lock);
	
	for(;;)
	{
		if(c2_net_object_read(C2_NET_OBJECT(imap), &buf) < 0)
		{
DIE:
			g_warning(_("Error reading from socket on IMAP host %s! Reader thread aborting!\n"),
						C2_NET_OBJECT(imap)->host);
			c2_imap_set_error(imap, NET_READ_FAILED);
			c2_net_object_disconnect(C2_NET_OBJECT(imap));
			c2_net_object_destroy_byte (C2_NET_OBJECT (imap));
			imap->state = C2IMAPDisconnected;
			gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
			if(final) g_free(final);
			return;
		}
		if(!final && strstr(buf, "FETCH (RFC822 {")) /* get FETCH replies right */
		{
			gint n, bytes = atoi(strstr(buf, "{") + 1);
			final = g_strdup(buf);
			while(bytes > 0)
			{
				if((n = c2_net_object_read(C2_NET_OBJECT(imap), &buf)) < 0)
					goto DIE;
				buf2 = final;
				final = g_strconcat(final, buf, NULL);
				g_free(buf2);
				g_free(buf);
				bytes -= n;
			}
			while( 1 )
			{
				if(c2_net_object_read(C2_NET_OBJECT(imap), &buf) < 0)
					goto DIE;
        if(strstr(buf, "FETCH completed"))
					break;
				buf2 = final;
				final = g_strconcat(final, buf, NULL);
				g_free(buf2);
				g_free(buf);
			}
		}
		if(!final) final = g_strdup(buf);
		else 
		{
			buf2 = final;
			final = g_strconcat(final, buf, NULL);
			g_free(buf2);
		}
		
		/* The IMAP server returned our tag, end of response */
		if(c2_strneq(buf, "CronosII-", 9) && (strstr(buf, "OK") 
			 || strstr(buf, "NO") || strstr(buf, "BAD")))
			break;
		g_free(buf);
	}
	
	tag = atoi(buf+9);
	g_free(buf);
  /* now insert 'final' into the hash...*/
	{
		C2IMAPServerReply *data = g_new0(C2IMAPServerReply, 1);
		data->tag = tag;
		data->value = final;
		imap->hash = g_list_append(imap->hash, data);
	}
	
	c2_imap_unlock_pending(imap);
	
	c2_mutex_unlock(&imap->lock);
}

/** c2_imap_unlock_pending
 * @imap: The IMAP object to operate on
 * 
 * This function will go trough the pending table
 * of the specified IMAP object and unlock any 
 * locked mutexes allowing blocking functions
 * awaiting data to continue.
 * 
 * Return Value:
 * None
 **/
static void
c2_imap_unlock_pending (C2IMAP *imap)
{
	GSList *ptr;
	GList *tmp;
	C2IMAPPending *pending;
	
	for(ptr = imap->pending; ptr; ptr = ptr->next)
	{
		C2IMAPServerReply *r;
		pending = ptr->data;
		for(tmp = imap->hash; tmp; tmp = tmp->next)
		{
			r = tmp->data;
			if(r->tag == pending->tag)
				c2_mutex_unlock(&pending->lock);
		}
	}
}

static void
c2_imap_remove_pending (C2IMAP *imap, tag_t tag)
{
  GSList *ptr;
	C2IMAPPending *pending;
	
	for(ptr = imap->pending; ptr; ptr = ptr->next)
	{
		pending = ptr->data;
		if(pending->tag == tag)
		{
			imap->pending = g_slist_remove_link(imap->pending, ptr);
			g_free(ptr->data);
			g_slist_free_1(ptr);
			return;
		}
	}
	
}

static tag_t
c2_imap_get_tag (C2IMAP *imap)
{
	tag_t tag;

	/* No, there's no error here, cmnd is an unsigned int of 10 bits,
	 * thus, the greatest number is 1023 and the lowest 0...
	 */
	
	tag = imap->cmnd++;

	return tag;
}

static void
c2_imap_set_error(C2IMAP *imap, const gchar *error)
{
	GtkObject *object = GTK_OBJECT(imap);
	gchar *buf;
	
	if((buf = gtk_object_get_data(object, "error")))
		g_free(buf);

	buf = g_strdup(error);
	gtk_object_set_data(object, "error", buf);
}

static C2IMAPPending *
c2_imap_new_pending (C2IMAP *imap, tag_t tag)
{
	C2IMAPPending *pending;
	
	pending = g_new0(C2IMAPPending, 1);
	pending->tag = tag;
	
	c2_mutex_init(&pending->lock);
	c2_mutex_lock(&pending->lock);
	
	imap->pending = g_slist_prepend(imap->pending, pending);
	
	return pending;
}

/**
 * c2_imap_get_server_reply
 * @imap: A locked IMAP object.
 * @tag: tag we are looking for
 *
 * This function will block and return the server
 * reply that is tagged with @tag.
 * 
 * Return Value:
 * A freeable string containing the server reply.
 **/
static gchar *
c2_imap_get_server_reply (C2IMAP *imap, tag_t tag)
{
	C2IMAPPending *pending;
	GList *tmp;
	gchar *data = NULL;

	pending = c2_imap_new_pending(imap, tag);
	
	/* just in case we got more server data */
	c2_imap_unlock_pending(imap);
	
	/* be careful to unlock the IMAP object before waiting on replies */	
	c2_mutex_unlock(&imap->lock);
	c2_mutex_lock(&pending->lock); /* wait for server reply... */
	c2_mutex_lock(&imap->lock);
	
	for(tmp = imap->hash; tmp; tmp = tmp->next)
	{
		C2IMAPServerReply *r = tmp->data;
		if(r->tag == tag)
		{
			data = r->value;
			imap->hash = g_list_remove_link(imap->hash, tmp);
			g_list_free_1(tmp);
			break;
		}
	}

	c2_mutex_unlock(&pending->lock);
	c2_mutex_destroy(&pending->lock);
	c2_imap_remove_pending(imap, tag);
	
	return data;
}

static gboolean
c2_imap_check_server_reply(gchar *reply, tag_t tag)
{
	gchar *ptr;
	
	/* quickie fix for single line reply checks */
	if(*reply != '*' && *reply != '+')
	{
		ptr = reply;
		ptr += C2TagLen;
		goto CHECK;
	}
	
	for(ptr = reply; *ptr; ptr++)
	{
		if(*ptr != '\n')
			continue;
		else if(c2_strneq(ptr+1, "CronosII-", 9))
		{
			ptr += C2TagLen + 1; /* skip '\nCronosII-XXXX ' */
CHECK:
			if(c2_strneq(ptr, "OK ", 3))
				return TRUE;
			else if(c2_strneq(ptr, "NO ", 3))
				return FALSE;
			else
			{
				g_warning(_("IMAP Server responded with 'BAD': %s\n"), reply);
				return FALSE;
			}
		}
	}
	
	g_warning(_("A server reply with no tagged ending sent to "
							"c2_imap_check_server_reply(): %s\n"), reply);
	return FALSE;
}

/** c2_imap_populate_folders
 * @imap: IMAP object in which to populate folders tree
 * 
 * Destroy's the old IMAP folder tree (if any) and creates
 * a new one.
 * 
 * Return Value:
 * 0 on success, -1 otherwise
 **/
gint
c2_imap_populate_folders (C2IMAP *imap)
{
	c2_mutex_lock(&imap->lock);
	
	if(imap->mailboxes)
		c2_mailbox_destroy_tree(imap->mailboxes);
	imap->mailboxes = NULL; 
	
	if(c2_imap_mailbox_loop(imap, NULL) < 0)
	{
		c2_mutex_unlock(&imap->lock);
		return -1;
	}
	
	c2_mutex_unlock(&imap->lock);

	gtk_signal_emit (GTK_OBJECT (imap), signals[MAILBOX_LIST], imap->mailboxes);
	return 0;
}

/** c2_imap_mailbox_loop
 * 
 * @imap: A locked IMAP Object
 * @parent: C2Mailbox for which to scan under for children,
 *          can be NULL, if scanning from top level of 
 *          folder hierarchy.
 * 
 * This is a recursive function which builds the 
 * imap->mailboxes folder tree.
 * 
 * Return Value:
 * 0 on success, -1 otherwise
 **/
static gint
c2_imap_mailbox_loop(C2IMAP *imap, C2Mailbox *parent)
{
	gchar *buf, *ptr, *start, *name;
	
	if(parent)
	{
		buf = c2_imap_get_full_mailbox_name(imap, parent);
		name = g_strconcat(buf, "/%", NULL);
		g_free(buf);
	}
	else name = g_strdup("%");
	
	if(!(buf = c2_imap_get_mailbox_list(imap, NULL, name)))
	{
		g_free(name);
		return -1;
	}
	g_free(name);
	
	start = buf;
	
	/* if this is a recursive call... */
	if(parent)
	{
		/* check to make sure there are child folders under this one
		 * and not that the server is just repeating itself w/ the 
		 * parent folder name */
		if((!imap->only_subscribed && c2_str_count_lines(buf) < 3) ||
			 (imap->only_subscribed && c2_str_count_lines(buf) < 2))
		{
				g_free(buf);
				return 0;
		}
		/* skip the first folder, which repeates itself on LIST commands */
		else if (!imap->only_subscribed)
		{
			while(start[0] != '\n') 
				start++;
			start++;
		}
	}
	
	for(ptr = start; *ptr; ptr++)
	{
		if(*start != '*')
			break;
		
		if(*ptr == '\n')
		{
			C2Mailbox *folder;
			gchar *buf2, *ptr2;
			guint num = 0;
			gchar *id = NULL, *tmp;
			gboolean noselect = FALSE, noinferiors = FALSE, marked = FALSE;
			if(parent) id = parent->id;
			
			buf2 = g_strndup(start, ptr - start);
			
			for(ptr2 = buf2; *ptr2; ptr2++)
			{
				if(num == 0 && *ptr2 == '(')
				{
					if(c2_strstr_case_insensitive(ptr2, "\\NoInferiors"))
						noinferiors = TRUE;
					if(c2_strstr_case_insensitive(ptr2, "\\NoSelect"))
						noselect = TRUE;
					if(c2_strstr_case_insensitive(ptr2, "\\Marked"))
						marked = TRUE;
					
					while(*ptr2 != ')' && *ptr2)
						ptr2++;
					num++;
				}
				else if(num > 0 && num < 3 && *ptr2 == ' ')
					num++;
				else if(num == 3)
				{
					if(*ptr2 == '"')
						name = g_strndup(ptr2+1, strlen(ptr2) - 3);
					else
						name = g_strndup(ptr2, strlen(ptr2) - 1);
					tmp = name;
					name = c2_imap_get_mailbox_hierarchy (name, 
						c2_imap_get_mailbox_level(name));
					g_free(tmp);
					/* check to see if this is a repeat... */
					if(parent && parent->child)
						folder = parent->child;
					else if(!parent && imap->mailboxes)
						folder = imap->mailboxes;
					else
						folder = NULL;
					for(; folder; folder = folder->next)
					{
						if(c2_streq(folder->name, name))
							goto NEXT;
					}
					folder = c2_mailbox_new_with_parent(&imap->mailboxes,
						name, id, C2_MAILBOX_IMAP, C2_MAILBOX_SORT_DATE, 
						GTK_SORT_ASCENDING, imap, FALSE);
					g_free(name);
					folder->protocol.IMAP.noinferiors = noinferiors;
					folder->protocol.IMAP.noselect = noselect;
					folder->protocol.IMAP.marked = marked;
					c2_db_load(folder);
					if(!folder->protocol.IMAP.noinferiors)
						if(c2_imap_mailbox_loop(imap, folder) < 0)
							return -1;
NEXT:
					num = 0;
					start = ptr+1;
					break;
				}
			}
		}
	}

	return 0;
}

static guint
c2_imap_get_mailbox_level (gchar *name)
{
	gchar *ptr;
	guint num = 1;
	
	for(ptr = name; *ptr; ptr++)
		if(*ptr == '/' && ptr != name) num++;
	
	return num;
}

static gchar *
c2_imap_get_mailbox_hierarchy (gchar *name, guint level)
{
	gchar *buf = NULL, *start, *end;
	
	for(start = name; *start; start++)
	{
		if(*start == '/' && start != name)
			level--;
		
		if(level <= 1)
		{
			if(start != name) start++;
			for(end = start; *end; end++)
				if(*end == '/')
					break;
			buf = g_strndup(start, end - start);
			break;
		}
	}
	
	return buf;
}

/* c2_imap_get_full_mailbox_name
 * @imap: C2IMAP object
 * @mailbox: Mailbox whose name is to be fetched
 * 
 * This function is used to find out the full path
 * of an imap mailbox in standard imap form:
 * "Inbox/mailing lists/software/CronosII"
 * 
 * Return Value:
 * A freeable string of the full name of the mailbox
 **/
static gchar *
c2_imap_get_full_mailbox_name (C2IMAP *imap, C2Mailbox *mailbox)
{
	C2Mailbox *temp;
	gint level = c2_mailbox_get_level(mailbox->id);
	gchar *id, *str, *name = NULL;

	if(level == 1)
		return g_strdup(mailbox->name);
	
	for( ; level > 1; level--)
	{
		id = c2_mailbox_get_parent_id(mailbox->id);
		temp = c2_mailbox_get_by_id(imap->mailboxes, id);
	 	g_free(id);
 		if(!name)
			name = g_strconcat(temp->name, "/", mailbox->name, NULL);
		else
	 	{
			str = g_strconcat(temp->name, "/", name, NULL);
		 	g_free(name);
			name = str;
		}
		mailbox = temp;
	}
	
	return name;
}

/** c2_imap_load_mailbox
 * 
 * @imap: A locked IMAP object
 * @mailbox: The mailbox to be loaded
 * 
 * Loads a linked list of C2Db's to represent
 * the messages present in the mailbox.
 * 
 * Return Value:
 * 0 on success, -1 otherwise
 **/
gint
c2_imap_load_mailbox (C2IMAP *imap, C2Mailbox *mailbox)
{
	/* TODO: 
	 * 	(done!) (1) Select box
	 *  (done!) (2) Build C2Db linked list according to messages 
	 *  (done!) (3) Close box 
	 **/
	tag_t tag;
	gint messages, uid = 0;
	gchar *reply, *ptr, *ptr2, *str;
	gchar *from = NULL, *subject = NULL, *date = NULL;
	time_t unixdate;
	gboolean seen = FALSE, answered = FALSE;
	C2Db *db;
	
	if((messages = c2_imap_select_mailbox(imap, mailbox)) < 0)
		return -1;
	
	if(messages == 0)
		goto CLOSE;
	
	tag = c2_imap_get_tag(imap);
	
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d FETCH 1:* "
			"(FLAGS UID BODY[HEADER.FIELDS (SUBJECT FROM DATE)])\r\n", tag) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return -1;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;
	
	for(ptr = reply, messages = 0; *ptr; ptr++)
	{
		if(*ptr == '*')
		{
			gint i = 0;
			ptr2 = ptr;
			while(*ptr2 != '\n' || !(*ptr2))
				ptr2++;
			str = g_strndup(ptr, ptr2 - ptr);
			if(c2_strstr_case_insensitive(str, "\\Seen"))
				seen = TRUE;
			if(c2_strstr_case_insensitive(str, "\\Answered"))
				answered = TRUE;
			if((ptr = strstr(str, "UID")))
				uid = atoi(ptr + 4);
			g_free(str);
			ptr = ptr2 + 1;
			for(i = 0; i < 3; i++)
			{
				gchar *ending;
				if(c2_strneq(ptr, "From: ", 6))
				{
					ptr += 6; /* strlen("From: ") + 1; */
					ending = strstr(ptr, "\r\n");
					from = g_strndup(ptr, ending - ptr);
					ptr += strlen(from) + 2;
					i++;
				}
				if(c2_strneq(ptr, "Subject: ", 9))
				{
					ptr += 9; /* strlen("Subject: ") + 1; */
					ending = strstr(ptr, "\r\n");
					subject = g_strndup(ptr, ending - ptr);
					ptr += strlen(subject) + 2;
					i++;
				}
				if(c2_strneq(ptr, "Date: ", 6))
				{
					ptr += 6; /* strlen("Date: ") + 1; */
					ending = strstr(ptr, "\r\n");
					date = g_strndup(ptr, ending - ptr);
					ptr += strlen(date) + 2;
					i++;
				}
			}

			if(date)
			{
				if ((unixdate = c2_date_parse(date)) == -1)
					if ((unixdate = c2_date_parse_fmt2 (date)) == -1)
						if ((unixdate = c2_date_parse_fmt3 (date)))
							unixdate = time (&unixdate);
			} else
				unixdate = time (&unixdate);
			/* FIX ME: the @account below should not be NULL */
			db = c2_db_new(mailbox, !seen, subject, from, NULL, unixdate, uid, messages);
			messages++;
			if(date) g_free(date);
			date = NULL;
			subject = NULL;
			from = NULL;
		}
	}
	
	g_free(reply);
	
CLOSE:
	if(c2_imap_close_mailbox(imap) < 0)
		return -1;
	
	return 0;
}

/** c2_imap_select_mailbox
 * 
 * @imap: A locked IMAP object
 * @mailbox: The mailbox to select
 * 
 * Performs a 'select' operation on the specified mailbox.
 * 
 * Return Value:
 * Number of messages in the box or -1 on failure
 **/
static gint
c2_imap_select_mailbox (C2IMAP *imap, C2Mailbox *mailbox)
{
	tag_t tag;
	gchar *reply, *ptr, *name, *line = NULL;
	gint retval;
	
	tag = c2_imap_get_tag(imap);
	
	if(mailbox->protocol.IMAP.noselect == TRUE)
	{
		c2_imap_set_error(imap, _("That IMAP mailbox is non-selectable"));
		return -1;
	}
	
	if(!(name = c2_imap_get_full_mailbox_name(imap, mailbox)))
	{
		c2_imap_set_error(imap, _("Invalid IMAP mailbox. Try re-starting CronosII and"
															" resycning your IMAP mailbox tree."));
		return -1;
	}
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d SELECT "
					"\"%s\"\r\n", tag, name) < 0)
	{
		g_free(name);
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return -1;
	}
	g_free(name);
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply+C2TagLen+3);
		g_free(reply);
		return -1;
	}
	
	ptr = reply;
	do
	{
		if(line) ptr += strlen(line);
		if(line) g_free(line);
		line = c2_str_get_line(ptr);
		if(!line)
		{
			c2_imap_set_error(imap, _("Invalid SELECT command status returned by IMAP server"));
			return -1;
		}
	}
	while(c2_strstr_case_insensitive(line, "EXISTS") == NULL);
	retval = atoi(line+2);
	g_free(line);
	
	imap->state = C2IMAPSelected;
	imap->selected_mailbox = mailbox;
	g_free(reply);
	return retval;
}

/**
 * c2_imap_close_mailbox
 * 
 * @imap: A locked IMAP object
 * 
 * Closes any currently opened (selected) mailbox
 * on IMAP session @imap.
 * 
 * Return Value:
 * 0 on success, -1 on failure
 **/
static gint
c2_imap_close_mailbox (C2IMAP *imap)
{
	tag_t tag;
	gchar *reply;
	
	if(imap->state < C2IMAPSelected)
		return 0;
	
	tag = c2_imap_get_tag(imap);
	
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d CLOSE\r\n", tag) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return -1;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply+C2TagLen+3);
		g_free(reply);
		return -1;
	}
	
	imap->state = C2IMAPAuthenticated;
	imap->selected_mailbox = NULL;
	g_free(reply);
	return 0;
}

static gint
c2_imap_plaintext_login (C2IMAP *imap)
{
	tag_t tag;
	gchar *reply;
	
	tag = c2_imap_get_tag (imap);
	
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d LOGIN %s %s\r\n", 
												tag, imap->user, imap->pass) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return -1;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;

	if(c2_imap_check_server_reply(reply, tag))
	{
		printf("logged in ok!\n\n");
	}
	else
		return -1;
	
	return 0;
}

/** 
 * c2_imap_get_mailbox_list
 * @imap: A locked IMAP object
 * @reference: folder reference
 * @name: folder name or wildcard
 * 
 * Returns the exact server output of the 
 * LSUB or LIST command after checking if it
 * was an OK reply
 * 
 * Return Value:
 * A freeable string of the LIST reply.
 **/
static gchar*
c2_imap_get_mailbox_list(C2IMAP *imap, const gchar *reference, const gchar *name)
{
	tag_t tag;
	gchar *reply, *cmd;
	
	tag = c2_imap_get_tag(imap);
	if(imap->only_subscribed) cmd = g_strdup("LSUB");
	else cmd = g_strdup("LIST");
	
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d %s \"%s\""
				" \"%s\"\r\n", tag, cmd, (reference) ? reference : "" , (name) ? name : "") < 0)
  {
		g_free(cmd);
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return NULL;
	}
	
	g_free(cmd);
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return NULL;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		g_free(reply);
		return NULL;
	}

	return reply;
}

/**
 * c2_imap_create_mailbox
 * @imap: A locked IMAP object.
 * @parent: A parent mailbox, or NULL for top-level folders
 * @name: Name of folder we want to delete
 *
 * This function will create the mailbox @name
 * 
 * Return Value:
 * The new C2Mailbox on success, NULL on failure
 **/
gboolean
c2_imap_create_mailbox(C2IMAP *imap, C2Mailbox *parent, const gchar *name)
{
	gchar *reply, *buf = NULL;
	tag_t tag;

	tag = c2_imap_get_tag(imap);

	if(parent) buf = c2_imap_get_full_mailbox_name(imap, parent);
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d CREATE "
		"\"%s%s%s\"\r\n", tag, buf ? buf : "", buf ? "/" : "", name) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		g_free(buf);
		return FALSE;
	}
	g_free(buf);
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return FALSE;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply+C2TagLen+3);
		g_free(reply);
		return FALSE;
	}

	return TRUE;
}

/**
 * c2_imap_delete_mailbox
 * @imap: A locked IMAP object.
 * @name: Name of folder we want to delete
 *
 * This function will delete the specified IMAP
 * folder in the full path of @name
 * 
 * Return Value:
 * 0 on success, -1 on failure
 **/
gboolean
c2_imap_delete_mailbox(C2IMAP *imap, C2Mailbox *mailbox)
{
	gchar *reply, *name;
	tag_t tag;
	
	tag = c2_imap_get_tag(imap);
	
	name = c2_imap_get_full_mailbox_name(imap, mailbox);
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d DELETE "
					"\"%s\"\r\n", tag, name) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		g_free(name);
		return FALSE;
	}
	g_free(name);
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return FALSE;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{	
		c2_imap_set_error(imap, reply + C2TagLen + 3);
		g_free(reply);
		return FALSE;
	}
	g_free(reply);
	
	return TRUE;
}

/**
 * c2_imap_rename_folder
 * @imap: The IMAP object.
 * @name: Name of folder we want to rename
 * @newname: New name of the folder
 *
 * This function will rename the specified IMAP
 * folder in the full path of @name
 * 
 * Return Value:
 * 0 on success, -1 on failure
 **/
#if 0
c2_imap_rename_folder(C2IMAP *imap, C2Mailbox *mailbox, gchar *name)
{
	gchar *reply, *oldname;
	tag_t tag;
	
	c2_mutex_lock(&imap->lock);
	
	tag = c2_imap_get_tag(imap);
	
	oldname = c2_imap_get_full_mailbox_name(imap, mailbox);
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d RENAME "
				"\"%s\" \"%s\"\r\n", oldname, name) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		c2_mutex_unlock(&imap->lock);
		g_free(oldname);
		return -1;
	}
	g_free(oldname);
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
	{
		c2_mutex_unlock(&imap->lock);
		return -1;
	}
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply + C2TagLen + 3);
		c2_mutex_unlock(&imap->lock);
		g_free(reply);
		return -1;
	}
	g_free(reply);
	
	g_free(mailbox->name);
	mailbox->name = g_strdup(name);
	
	c2_mutex_unlock(&imap->lock);
	
	return 0;
}
#endif

/* c2_imap_subscribe_mailbox
 * 
 * @imap: An IMAP object
 * @mailbox: Mailbox to subscribe to
 * 
 * Will do an IMAP 'subscribe' to the mailbox.
 * 
 * Return Value:
 * TRUE on success, FALSE otherwise.
 **/
gboolean
c2_imap_subscribe_mailbox (C2IMAP *imap, C2Mailbox *mailbox)
{
	gchar *reply;
	tag_t tag;
	
	c2_mutex_lock(&imap->lock);
	tag = c2_imap_get_tag(imap);
	
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d SUBSCRIBE "
			"\"%s\"\r\n", tag, c2_imap_get_full_mailbox_name(imap, mailbox)) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
	{
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply+C2TagLen + 3);
		g_free(reply);
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	g_free(reply);
	c2_mutex_unlock(&imap->lock);

	return TRUE;
}

/* c2_imap_subscribe_mailbox
 * 
 * @imap: An IMAP object
 * @mailbox: Mailbox to unsubscribe to
 * 
 * Will do an IMAP 'unsubscribe' to the mailbox.
 * 
 * Return Value:
 * TRUE on success, FALSE otherwise.
 **/
gboolean
c2_imap_unsubscribe_mailbox (C2IMAP *imap, C2Mailbox *mailbox)
{
	gchar *reply;
	tag_t tag;
	
	c2_mutex_lock(&imap->lock);
	tag = c2_imap_get_tag(imap);
	
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d UNSUBSCRIBE "
			"\"%s\"\r\n", tag, c2_imap_get_full_mailbox_name(imap, mailbox)) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
	{
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply+C2TagLen + 3);
		g_free(reply);
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	g_free(reply);
	c2_mutex_unlock(&imap->lock);

	return TRUE;
}

gboolean
c2_imap_mailbox_is_subscribed (C2IMAP *imap, C2Mailbox *mailbox)
{
	gchar *reply, *name;
	tag_t tag;
	
	c2_mutex_lock(&imap->lock);
	tag = c2_imap_get_tag(imap);
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d LSUB \"\" \""
		 "%s\"\r\n", tag, c2_imap_get_full_mailbox_name(imap, mailbox)) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
	{
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		g_free(reply);
		c2_mutex_unlock(&imap->lock);
		return FALSE;
	}
	
	name = g_strconcat(c2_imap_get_full_mailbox_name(imap, mailbox), "\r\n", NULL);
	g_free(reply);
	if(strstr(reply, name))
	{
		g_free(name);
		c2_mutex_unlock(&imap->lock);
		return TRUE;
	}
	else
	{
		g_free(name);
		c2_mutex_unlock(&imap->lock);
		return FALSE;	
	}
}

/** c2_imap_message_remove
 * 
 * @imap: A locked IMAP object
 * @db: Message to remove
 * 
 * Will expunge a message.
 * 
 * Return Value:
 * 0 on success, -1 otherwise
 **/
gint
c2_imap_message_remove (C2IMAP *imap, GList *list)
{
	C2Db *db;
	gchar *reply;
	tag_t tag;
	
	for( ; list; list = list->next)
	{
		db = list->data;
		if(imap->selected_mailbox != db->mailbox)
			if(c2_imap_select_mailbox(imap, db->mailbox) < 0)
	      return -1;
		
		tag = c2_imap_get_tag(imap);
		if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d STORE "
			 "%i +FLAGS.SILENT (\\Deleted)\r\n", tag, db->position+1) < 0)
		{
			c2_imap_set_error(imap, NET_WRITE_FAILED);
			imap->state = C2IMAPDisconnected;
			gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
			return -1;
		}
	
		if(!(reply = c2_imap_get_server_reply(imap, tag)))
			return -1;

		if(!c2_imap_check_server_reply(reply, tag))
		{
			c2_imap_set_error(imap, reply + C2TagLen + 3);
			g_free(reply);
			return -1;
		}
		g_free(reply);
	}
	
	tag = c2_imap_get_tag(imap);
  if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d EXPUNGE\r\n",
		 tag) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return -1;
	}
	
  if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;
	
	if(!c2_imap_check_server_reply(reply, tag)) 
	{      
		c2_imap_set_error(imap, reply + C2TagLen + 3);     
		g_free(reply);     
		return -1;
	}  
  g_free(reply);
	
	c2_imap_close_mailbox(imap);
	
	return 0;
}


/** c2_imap_message_add
 *
 * @imap: A locked IMAP object
 * @mailbox: Mailbox to which to add the message to
 * @db: Message to add
 *
 * Will add @db to @mailbox.
 *
 * Return Value:
 * 0 on success, -1 otherwise
 **/
gint
c2_imap_message_add (C2IMAP *imap, C2Mailbox *mailbox, C2Db *db)
{
	gchar *reply;
	tag_t tag;
	gint32 size = strlen(db->message->header) + strlen(db->message->body) + 3;
	gint messages;

	tag = c2_imap_get_tag(imap);
	if((c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d APPEND "
		 "\"%s\" () {%d}\r\n", tag, mailbox->name, size) < 0) ||
		 (c2_net_object_send(C2_NET_OBJECT(imap), NULL, "%s\r\n%s\r\n",
			db->message->header, db->message->body) < 0))
	{
		 		c2_imap_set_error(imap, NET_WRITE_FAILED);
		 		imap->state = C2IMAPDisconnected;
		 		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		 		return -1;
	}
	
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;

	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply + C2TagLen + 3);
 		g_free(reply);
 		return -1;
	}
	g_free(reply);
	
	tag = c2_imap_get_tag(imap);
	if((messages = c2_imap_select_mailbox(imap, mailbox)) < 0)
		return -1;
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d FETCH "
		 "%i (UID)\r\n", tag, messages) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return -1;
	}
	c2_imap_close_mailbox(imap);
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return -1;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply + C2TagLen + 3);
		g_free(reply);
		return -1;
	}
	db->mid = atoi(strstr(reply, "UID ") + 4);
	g_free(reply);

	return 0;
}

/** c2_imap_message_load
 * 
 * @imap: A locked IMAP object
 * @db: C2Db of message to load
 * 
 * Loads and returns the requested email 
 * message.
 * 
 * Return Value:
 * The new C2Message or NULL on failure.
 **/
C2Message *
c2_imap_load_message (C2IMAP *imap, C2Db *db)
{
	C2Message *message;
	gchar *reply, *start, *end;
	tag_t tag;
	
	if(c2_imap_select_mailbox(imap, db->mailbox) < 0)
		return NULL;
	
	tag = c2_imap_get_tag(imap);
	if(c2_net_object_send(C2_NET_OBJECT(imap), NULL, "CronosII-%04d FETCH %i "
		 "(RFC822)\r\n", tag, db->position+1) < 0)
	{
		c2_imap_set_error(imap, NET_WRITE_FAILED);
		imap->state = C2IMAPDisconnected;
		gtk_signal_emit(GTK_OBJECT(imap), signals[NET_ERROR]);
		return NULL;
	}
	
	if(!(reply = c2_imap_get_server_reply(imap, tag)))
		return NULL;
	
	if(!c2_imap_check_server_reply(reply, tag))
	{
		c2_imap_set_error(imap, reply + C2TagLen + 3);
		g_free(reply);
		return NULL;
	}
	
	if(!(start = strstr(reply, "\r\n")))
	{
		g_free(reply);
		return NULL;
	}
	start += 2;
	if(!(end = strstr(start, "\r\n\r\n")))
		if(!(end = strstr(start, ")\r\nCronosII-")))
		{
			g_free(reply);
			return NULL;
		}
	
	message = c2_message_new();
	message->header = g_strndup(start, end - start);
	start = end + 4;
	end = strstr(start, "OK FETCH completed");
	end -= (C2TagLen + 5); /* C2TagLen + '\r\n)\r\n' */
	message->body = g_strndup(start, end - start);
	
	g_free(reply);
	return message;
}
