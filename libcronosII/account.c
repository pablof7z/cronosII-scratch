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
#include <glib.h>
#include <stdarg.h>

#include "account.h"
#include "error.h"
#include "pop3.h"
#include "smtp.h"
#include "utils.h"

static void
class_init										(C2AccountClass *klass);

static void
init											(C2Account *account);

static void
destroy											(GtkObject *object);

static void
set_data									(C2Account *account, C2AccountKey key, gint type, gpointer data);

static gpointer
get_data									(C2Account *account, C2AccountKey key, gint *type);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_account_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info = {
			"C2Account",
			sizeof (C2Account),
			sizeof (C2AccountClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2AccountClass *klass)
{
	GtkObjectClass *object_class;
	object_class = (GtkObjectClass *) klass;
	parent_class = gtk_type_class (gtk_object_get_type ());
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	object_class->destroy = destroy;
}

static void
init (C2Account *account)
{
	account->name = NULL;
	account->email = NULL;
	account->edata = NULL;
	account->etype = NULL;
	account->next = NULL;
}

C2Account *
c2_account_new (C2AccountType type, gchar *name, gchar *email)
{
	C2Account *account;

	c2_return_val_if_fail (name, NULL, C2EDATA);
	c2_return_val_if_fail (email, NULL, C2EDATA);

	account = gtk_type_new (c2_account_get_type ());

	account->type = type;
	account->name = name;
	account->email = email;

	return account;
}

void
c2_account_set_extra_data (C2Account *account, C2AccountKey key, gint type, gpointer data)
{
	c2_return_if_fail (account, C2EDATA);
	
	set_data (account, key, type, data);
}

gpointer
c2_account_get_extra_data (C2Account *account, C2AccountKey key, gint *type)
{
	c2_return_val_if_fail (account, NULL, C2EDATA);

	return get_data (account, key, type);
}

static void
set_data (C2Account *account, C2AccountKey key, gint type, gpointer data)
{
	GSList *l;
	gint i;

	for (l = account->edata, i = 0; l; l = g_slist_next (l), i++)
	{
		if (GPOINTER_TO_INT (l->data) == key)
		{
			GSList *r;

			r = g_slist_nth (account->etype, i);
			account->etype = g_slist_remove_link (account->etype, r);
			account->edata = g_slist_remove_link (account->edata, l);
			break;
		}
	}
	
	account->edata = g_slist_prepend (account->edata, (gpointer) key);
	account->etype = g_slist_prepend (account->etype, (gpointer) type);

	gtk_object_set_data (GTK_OBJECT (account), (gpointer) &key, data);
}

static gpointer
get_data (C2Account *account, C2AccountKey key, gint *type)
{
	GSList *l;
	gint i;
	gpointer value;
L
	value = gtk_object_get_data (GTK_OBJECT (account), (gpointer) &key);

	for (l = account->edata, i = 0; l; l = g_slist_next (l), i++)
	{
		if (GPOINTER_TO_INT (l->data) == key)
		{
			GSList *r;

			if (type)
			{
				r = g_slist_nth (account->etype, i);
				if (r)
					*type = GPOINTER_TO_INT (r->data);
			}
			break;
		}
	}

L	return value;
}

static void
destroy (GtkObject *object)
{
	GSList *l;
	C2Account *account = C2_ACCOUNT (object);
	
	g_free (account->name);
	g_free (account->email);

	for (l = account->edata; l; l = g_slist_next (l))
	{
		gpointer value;
		gint type;

		value = get_data (account, GPOINTER_TO_INT (l->data), &type);

		switch (type)
		{
			case GTK_TYPE_OBJECT:
				gtk_object_unref (GTK_OBJECT (value));
				break;
			default:
				g_free (value);
		}
	}
}

void
c2_account_free_all (C2Account *head)
{
	C2Account *l, *s;
	
	for (l = head; l != NULL;)
	{
		s = l->next;
		gtk_object_destroy (GTK_OBJECT (l));
		l = s;
	}
}

/**
 * c2_account_append
 * @head: A C2Account object.
 * @obj: The object that wants to be appended.
 *
 * This function will append @obj to @head.
 *
 * Return Value:
 * The new linked list with @obj at the end.
 **/
C2Account *
c2_account_append (C2Account *head, C2Account *obj)
{
	C2Account *s;
	
	c2_return_val_if_fail (obj, head, C2EDATA);

	if (!(s = c2_account_last (head)))
		return obj;
	else
		s->next = obj;

	return head;
}

/**
 * c2_account_get_by_name
 * @head: A C2Account object where to start looking.
 * @name: Name of mailbox to get.
 *
 * This function searchs the list @head looking
 * for the mailbox with name @name.
 *
 * Return Value:
 * The C2Mailbox that mached the name or %NULL.
 **/
C2Account *
c2_account_get_by_name (C2Account *head, const gchar *name)
{
	C2Account *l;

	for (l = head; l; l = c2_account_next (l))
		if (c2_streq (l->name, name))
			break;

	return l;
}

/**
 * c2_account_last
 * @head: A C2Account object list.
 *
 * This function will find the last element
 * of a C2Account linked list.
 *
 * Return Value:
 * The last element of the linked list.
 **/
C2Account *
c2_account_last (C2Account *head)
{
	C2Account *s, *c;

	for (s = c = head; s; s = c2_account_next (s))
		if (c != s)
			c = c2_account_next (c);

	return c;
}

/**
 * c2_account_check
 * @account: The C2Account object to work on.
 *
 * This function will start the checking
 * of an account.
 *
 * Return Value:
 * 0 on success, -1 on error.
 **/
gint
c2_account_check (C2Account *account)
{
	typedef gint (*FetchmailFunction) (C2Account *);
	FetchmailFunction fetchmail;
	
	switch (account->type)
	{
		case C2_ACCOUNT_POP3:
			fetchmail = (FetchmailFunction) c2_pop3_fetchmail;
			break;
		case C2_ACCOUNT_IMAP:
			printf ("No one developed the IMAP part of C2: %s:%d\n", __FILE__, __LINE__);
			break;
		default:
			g_warning ("Account is misconfigured, type = %d\n", account->type);
	}

	return fetchmail (account);
}
