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

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

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
c2_account_check (const C2Account *account)
{
	typedef gint (*FetchmailFunction) (const C2Account *);
	FetchmailFunction fetchmail;
	
	switch (account->type)
	{
		case C2_ACCOUNT_POP3:
			fetchmail = (FetchmailFunction) c2_pop3_fetchmail;
			break;
	}

	return fetchmail (account);
}

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
	account->per_name = NULL;
	account->organization = NULL;
	account->email = NULL;
	account->reply_to = NULL;
	account->protocol.pop3 = NULL;
	account->smtp = NULL;
	account->signature.string = NULL;
	account->next = NULL;
}

C2Account *
c2_account_new (const gchar *name, const gchar *per_name, const gchar *organization, const gchar *email,
				const gchar *reply_to, gboolean active, C2AccountType account_type,
				C2SMTPType smtp_type, C2AccountSignatureType signature_type, const gchar *signature,
				gboolean signature_automatic, ...)
{
	C2Account *account;
	const gchar *user, *pass, *host;
	gint flags;
	gboolean smtp_authentication;
	gint port;
	gchar *file;
	va_list args;

	c2_return_val_if_fail (name || email, NULL, C2EDATA);

	va_start (args, signature_automatic);
	
	account = gtk_type_new (C2_TYPE_ACCOUNT);
	account->name = g_strdup (name);
	account->per_name = g_strdup (per_name);
	account->organization = g_strdup (organization);
	account->email = g_strdup (email);
	account->reply_to = g_strdup (reply_to);	
	account->options.active = active;

	account->type = account_type;

	switch (account->type)
	{
		case C2_ACCOUNT_POP3:
			host = va_arg (args, const gchar *);
			port = va_arg (args, gint);
			user = va_arg (args, const gchar *);
			pass = va_arg (args, const gchar *);
			flags = va_arg (args, gint);
			
			account->protocol.pop3 = c2_pop3_new (user, pass, host, port);
			c2_pop3_set_flags (account->protocol.pop3, flags);
			break;
	}
	
	switch (smtp_type)
	{
		case C2_SMTP_REMOTE:
			host = va_arg (args, gchar *);
			port = va_arg (args, gint);
			smtp_authentication = va_arg (args, gboolean);
			user = va_arg (args, gchar *);
			pass = va_arg (args, gchar *);
			
			account->smtp = c2_smtp_new (C2_SMTP_REMOTE, host, port, smtp_authentication, user, pass);
			break;
		case C2_SMTP_LOCAL:
			account->smtp = c2_smtp_new (C2_SMTP_LOCAL);
			break;
	}
	
	account->signature.type = signature_type;
	account->signature.string = g_strdup (signature);
	account->signature.automatic = signature_automatic;

	va_end (args);

	return account;
}

static void
destroy (GtkObject *object)
{
	C2Account *account = C2_ACCOUNT (object);
	g_free (account->name);
	g_free (account->per_name);
	g_free (account->organization);
	g_free (account->email);
	g_free (account->reply_to);
	if (account->type == C2_ACCOUNT_POP3)
		gtk_object_destroy (GTK_OBJECT (account->protocol.pop3));
	c2_smtp_free (account->smtp);
	g_free (account->signature.string);
	g_free (account);
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
 * c2_account_copy
 * @account: C2Account object to copy.
 *
 * This function will copy a C2Account object.
 * 
 * Return Value:
 * The copy of the object.
 **/
C2Account *
c2_account_copy (C2Account *account)
{
	C2Account *copy = NULL;
	
	c2_return_val_if_fail (account, NULL, C2EDATA);

	if (account->type == C2_ACCOUNT_POP3)
	{
		switch (account->smtp->type)
		{
			case C2_SMTP_REMOTE:
				copy = c2_account_new (account->name, account->per_name, account->organization,
										account->email, account->reply_to, account->options.active,
										account->type, account->smtp->type, 
										account->signature.type,
										account->signature.string,
										account->signature.automatic,
										C2_NET_OBJECT (account->protocol.pop3)->host,
										C2_NET_OBJECT (account->protocol.pop3)->port,
										account->protocol.pop3->user, account->protocol.pop3->pass,
										account->protocol.pop3->flags,
										account->smtp->host, account->smtp->port,
										account->smtp->authentication, account->smtp->user,
										account->smtp->pass);
				break;
			case C2_SMTP_LOCAL:
				copy = c2_account_new (account->name, account->per_name, account->organization,
										account->email, account->reply_to, account->options.active,
										account->type, account->smtp->type, 
										account->signature.type,
										account->signature.string,
										account->signature.automatic,
										C2_NET_OBJECT (account->protocol.pop3)->host,
										C2_NET_OBJECT (account->protocol.pop3)->port,
										account->protocol.pop3->user, account->protocol.pop3->pass,
										account->protocol.pop3->flags);
		}
		
		c2_smtp_set_flags (copy->smtp, account->smtp->flags);
	}

	return copy;
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

	return NULL;
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
