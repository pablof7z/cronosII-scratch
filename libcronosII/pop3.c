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
#include <unistd.h>
#include <string.h>
#include <config.h>

#include "i18n.h"
#include "error.h"
#include "net-object.h"
#include "pop3.h"
#include "utils.h"
#include "md5.h"

/*
 * [TODO] Add support for CRAM-MD5 authentication.
 * [DONE] Add APOP support.
 */

#define DEFAULT_FLAGS C2_POP3_DO_NOT_KEEP_COPY | C2_POP3_DO_NOT_LOSE_PASSWORD

#define UIDL_LENGTH		70

static void
class_init									(C2POP3Class *klass);

static void
init										(C2POP3 *pop3);

static void
destroy										(GtkObject *object);

static gint
welcome										(C2POP3 *pop3);

static gint
login										(C2POP3 *pop3);

static gint
login_apop									(C2POP3 *pop3);

static gint
login_plain									(C2POP3 *pop3);

static GSList *
status										(C2POP3 *pop3, C2Account *account, GSList **cuidl);

static gboolean
uidl_in_db									(C2Account *account, const gchar *uidl);

static void
uidl_clean									(C2Account *account);

static void
uidl_add									(C2Account *account, const gchar *uidl, time_t date);

static gint
retrieve									(C2POP3 *pop3, C2Account *account,
											 C2Mailbox *inbox, GSList *download_list);

static gint
synchronize									(C2POP3 *pop3, C2Account *account, GSList *uidl);

static void
quit										(C2POP3 *pop3);

enum
{
	LOGIN,
	LOGIN_FAILED,
	UIDL,
	STATUS,
	RETRIEVE,
	SYNCHRONIZE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObject *parent_class = NULL;

GtkType
c2_pop3_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info = {
			"C2POP3",
			sizeof (C2POP3),
			sizeof (C2POP3Class),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (c2_net_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2POP3Class *klass)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (c2_net_object_get_type ());

	signals[LOGIN] =
		gtk_signal_new ("login",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2POP3Class, login),
					gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[UIDL] =
		gtk_signal_new ("uidl",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2POP3Class, uidl),
					gtk_marshal_NONE__INT_INT, GTK_TYPE_NONE, 2,
					GTK_TYPE_INT, GTK_TYPE_INT);
	signals[STATUS] =
		gtk_signal_new ("status",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2POP3Class, status),
					gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
					GTK_TYPE_INT);
	signals[RETRIEVE] =
		gtk_signal_new ("retrieve",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2POP3Class, retrieve),
					c2_marshal_NONE__INT_INT_INT, GTK_TYPE_NONE, 3,
					GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_INT);
	signals[SYNCHRONIZE] =
		gtk_signal_new ("synchronize",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2POP3Class, synchronize),
					gtk_marshal_NONE__INT_INT, GTK_TYPE_NONE, 2,
					GTK_TYPE_INT, GTK_TYPE_INT);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->login = NULL;
	klass->login_failed = NULL;
	klass->uidl = NULL;
	klass->status = NULL;
	klass->retrieve = NULL;
	klass->synchronize = NULL;
	object_class->destroy = destroy;
}

static void
init (C2POP3 *pop3)
{
	pop3->user = pop3->pass = NULL;
	pop3->flags = DEFAULT_FLAGS;
	pthread_mutex_init (&pop3->run_lock, NULL);
}

static void
destroy (GtkObject *object)
{
	C2POP3 *pop3 = C2_POP3 (object);

	if (c2_net_object_is_offline (C2_NET_OBJECT (pop3)))
#ifdef USE_DEBUG
	{
		g_print ("A C2POP3 object is being freed while a connection is "
				 "being used (%s)!\n", pop3->user);
#endif
		c2_net_object_disconnect (C2_NET_OBJECT (pop3));
#ifdef USE_DEBUG
	}
#endif
	g_free (pop3->user);
	g_free (pop3->pass);
	pthread_mutex_destroy (&pop3->run_lock);
}

/**
 * c2_pop3_new
 * @host: Hostame.
 * @port: Port.
 * @user: Username.
 * @pass: Password (might be NULL).
 * @ssl: Wheter SSL connection should be used or not.
 *
 * This function will create a new C2POP3 with
 * the data you pass it and with some default configuration.
 * 
 * Return Value:
 * The allocated C2POP3 object or %NULL if there was an error.
 **/
C2POP3 *
c2_pop3_new (gchar *host, gint port, gchar *user, gchar *pass, gboolean ssl)
{
	C2POP3 *pop3;
	
	c2_return_val_if_fail (user || host, NULL, C2EDATA);
	
	pop3 = gtk_type_new (C2_TYPE_POP3);

	pop3->user = user;
	pop3->pass = pass;

	c2_net_object_construct (C2_NET_OBJECT (pop3), host, port, ssl);

	return pop3;
}

/**
 * c2_pop3_set_flags
 * @pop3: An allocated C2POP3 object.
 * @flags: Flags to set in the object.
 *
 * This function will force to change the flags
 * of a C2Pop3 object.
 * Flags let the object customize according
 * to the users preferences.
 **/
void
c2_pop3_set_flags (C2POP3 *pop3, gint flags)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->flags = flags;
}

void
c2_pop3_set_auth_method (C2POP3 *pop3, C2POP3AuthenticationMethod auth_method)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->auth_method = auth_method;
}

void
c2_pop3_set_leave_copy (C2POP3 *pop3, gboolean leave_copy, gint days)
{
	c2_return_if_fail (pop3, C2EDATA);

	if (leave_copy)
		pop3->flags |= C2_POP3_DO_KEEP_COPY;
	else
		pop3->flags |= C2_POP3_DO_NOT_KEEP_COPY;

	if (leave_copy)
		pop3->copies_in_server_life_time = ((days)*(86000));
	else
		pop3->copies_in_server_life_time = 0;
}

/**
 * c2_pop3_fetchmail
 * @pop3: POP3 object to check.
 * @account: Account where the POP3 object belongs.
 * @inbox: Inbox where to store downloaded mails.
 *
 * This function will download
 * messages from the account @account using
 * the POP3 protocol.
 *
 * Return Value:
 * 0 on success or -1.
 **/
gint
c2_pop3_fetchmail (C2POP3 *pop3, C2Account *account, C2Mailbox *inbox)
{
	GSList *download_list = NULL, *uidl_list;
	gint mails;
	gint retval = 0;

	c2_return_val_if_fail (pop3, -1, C2EDATA);

	/* Lock the mutex */
	pthread_mutex_lock (&pop3->run_lock);
	
	gtk_object_set_data (GTK_OBJECT (pop3), "account", account);

	if (c2_net_object_run (C2_NET_OBJECT (pop3)) < 0)
	{	
		retval = -1;
		goto shutdown;
	}

	if (welcome (pop3) < 0)
	{
		retval = -1;
		goto after_quit;
	}

	if (login (pop3) < 0)
	{
		retval = -1;
		goto after_quit;
	}

	if ((download_list = status (pop3, account, &uidl_list)) < 0)
	{
		retval = -1;
		goto after_quit;
	}

	if (retrieve (pop3, account, inbox, download_list) < 0)
	{
		retval = -1;
		goto after_quit;
	}

	if (pop3->flags & C2_POP3_DO_KEEP_COPY && pop3->copies_in_server_life_time)
	{
		if (synchronize (pop3, account, uidl_list))
		{
			retval = -1;
			goto after_quit;
		}
	} else
	{
		GSList *l;

		for (l = uidl_list; l; l = g_slist_next (l))
			g_free (l->data);
		g_slist_free (uidl_list);
	}

	quit (pop3);
	
after_quit:
	printf("done with fetching\n");

	if (!retval)
		c2_net_object_disconnect (C2_NET_OBJECT (pop3));
	else
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
	
	g_slist_free (download_list);
	
shutdown:

	/* Unlock the mutex */
	gtk_object_remove_data (GTK_OBJECT (pop3), "account");
	pthread_mutex_unlock (&pop3->run_lock);

	return retval;
}

static gint
welcome (C2POP3 *pop3)
{
	gchar *string = NULL;
	gchar *logintokenpos = NULL;
	gchar *logintoken = NULL;
	gint  loginsize = 0;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return -1;

	if (c2_strnne (string, "+OK", 3))
	{

		string = strstr (string, " ");
		if (string)
			string++;

		c2_error_object_set_custom (GTK_OBJECT (pop3), string);
		return -1;
	}

	if (pop3->auth_method == C2_POP3_AUTHENTICATION_APOP)
	{
		gchar *ptr;
		size_t size;
		
		ptr = strstr (string,"<");
		if (ptr == NULL)
		{
			c2_error_object_set_custom (GTK_OBJECT (pop3), _("Server does not support APOP"));
			return -1;
		}

		size = (strstr (ptr+1, ">")+1)-ptr;
		pop3->logintoken = g_strndup (ptr, size);
	} else
		pop3->logintoken = NULL;

	return 0;
}

static gint
login (C2POP3 *pop3)
{
	gchar *string;
	gint i = 0;
	gboolean logged_in = FALSE;
	
	gtk_signal_emit (GTK_OBJECT (pop3), signals[LOGIN]);

	do
	{
		gint retval;
		
		if (pop3->auth_method == C2_POP3_AUTHENTICATION_APOP)
			retval = login_apop (pop3);
		else
			retval = login_plain (pop3);
		
		if (retval == -1)
			return -1;
		if (retval == 0)
			logged_in = FALSE;
		else
			logged_in = TRUE;

		if (!logged_in)
		{
			const gchar *error;
			gchar *newuser, *newpass;
			gboolean ret;

			error = c2_error_object_get (GTK_OBJECT (pop3));
			
			newuser = NULL;
			newpass = NULL;
			if (C2_POP3_CLASS_FW (pop3)->login_failed)
			{
				pthread_mutex_t lock;
				gint cont;
				
				pthread_mutex_init (&lock, NULL);
				pthread_mutex_lock (&lock);
				cont = C2_POP3_CLASS_FW (pop3)->login_failed (pop3, error, &newuser, &newpass, &lock);
				pthread_mutex_lock (&lock);
				pthread_mutex_unlock (&lock);
				pthread_mutex_destroy (&lock);
				if (cont)
				{
					pop3->user = newuser;
					pop3->pass = newpass;
				} else
				{
					c2_error_object_set (GTK_OBJECT (pop3), C2USRCNCL);
					return -1;
				}
			} else
			{
				c2_error_object_set_custom (GTK_OBJECT (pop3), string);
				return -1;
			}
		}
	} while (++i < 3 && !logged_in && pop3->pass);
	
	if (!logged_in)
		return -1;

	return 0;
}

static gboolean
login_apop (C2POP3 *pop3)
{
	gchar *string;
	gchar *apopstring;
	unsigned char md5apop[16];
	gchar md5apopstring[33];
	int x;
	
	/* allocate a string for the pass+logintoken so we can get the md5 hash of it */
	apopstring = g_strconcat (pop3->logintoken, pop3->pass, NULL);

	md5_buffer (apopstring, strlen(apopstring), md5apop);

	/* print out the md5 hash */
	for( x = 0; x < 16; x++)
		sprintf (&md5apopstring[x*2],"%02x",md5apop[x]);
	md5apopstring[32] = 0;

	if (c2_net_object_send (C2_NET_OBJECT (pop3), "APOP %s %s\r\n", pop3->user,md5apopstring) < 0)
	{
		c2_error_object_set_custom(GTK_OBJECT (pop3), "Sending APOP login failed");
			return -1;
	}
	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
	{
		c2_error_object_set_custom(GTK_OBJECT (pop3), "Failed to recv APOP login reply");
		return -1;
	}

	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;
		c2_error_object_set_custom (GTK_OBJECT (pop3), string);
		return 0;
	} else
		return 1;
}

/**
 * login_plain
 * @pop3: C2POP3 to login.
 *
 * This function login using the plain type of login.
 *
 * Return Value:
 * Indicates which is the status of the login:
 * -1: Socket error, connection should be aborted
 *     immediatly.
 *  0: Login failed, connection might keep alive.
 * +1: Login successfull.
 **/
static gint
login_plain (C2POP3 *pop3)
{
	gchar *string;
	
	/* Username */
	if (c2_net_object_send (C2_NET_OBJECT (pop3), "USER %s\r\n", pop3->user) < 0)
		return -1;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return -1;

	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;
		c2_error_object_set_custom (GTK_OBJECT (pop3), string);
		return 0;
	}

	/* Password */
	g_free (string);
	if (c2_net_object_send (C2_NET_OBJECT (pop3), "PASS %s\r\n", pop3->pass) < 0)
		return -1;
	
	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return -1;
	
	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;
		c2_error_object_set_custom (GTK_OBJECT (pop3), string);
		return 0;
	} else
		return 1;

	return 0;
}

/* [TODO]
 * Perhaps this function should return the classic gint
 * value indicating if things went well or wrong and return
 * the GSList as a double-pointer.
 */
static GSList *
status (C2POP3 *pop3, C2Account *account, GSList **cuidl)
{
	gchar *string;
	gint mails;
	GSList *list = NULL;

	*cuidl = NULL;

	if (c2_net_object_send (C2_NET_OBJECT (pop3), "STAT\r\n") < 0)
		return NULL;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return NULL;

	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;

		c2_error_object_set_custom (GTK_OBJECT (pop3), string);
		return NULL;
	}

	sscanf (string, "+OK %d ", &mails);

	/* Bosko's suggestion. 08/09/01 --pablo */
	if (!mails)
		uidl_clean (account);
	else
	{
		if (pop3->flags & C2_POP3_DO_KEEP_COPY)
		{
			gint i;
			
			gtk_signal_emit (GTK_OBJECT (pop3), signals[UIDL], 0, mails);
			
			if (c2_net_object_send (C2_NET_OBJECT (pop3), "UIDL\r\n") < 0)
				return NULL;
			
			if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
				return NULL;
			
			if (c2_strnne (string, "+OK", 3))
			{
				g_free (string);
				goto no_uidl;
			}
			g_free (string);
			
			for (i = 1; ; i++)
			{
				gchar *prompt;
				gchar uidl[UIDL_LENGTH] = { 0 };
				
				if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
					return NULL;
				
				prompt = g_strdup_printf ("%d %%s\r\n", i);
				sscanf (string, prompt, uidl);
				
				if (!i || !strlen (uidl))
				{
					g_free (prompt);
					g_free (string);
					break;
				}

				/* We have a Unique-ID, add it to the
				 * cuidl list, no matter what...
				 */
				*cuidl = g_slist_append (*cuidl, (gpointer) g_strdup (string));
				
				/* We have the Unique-ID, check if
				 * we already have it in our database.
			 	 */
				if (!uidl_in_db (account, uidl))
					list = g_slist_append (list, (gpointer) string);
				else
					g_free (string);

				gtk_signal_emit (GTK_OBJECT (pop3), signals[UIDL], i, mails);
		
				g_free (prompt);
			}
		} else
		{
			/* Append every message to the download list */
			gint i;
	
no_uidl:
			for (i = 1; i <= mails; i++)
				list = g_slist_append (list, (gpointer) g_strdup_printf ("%d", i));
		}
	}
	
	mails = g_slist_length (list);
	gtk_signal_emit (GTK_OBJECT (pop3), signals[STATUS], mails);

	return list;
}

static gboolean
uidl_in_db (C2Account *account, const gchar *uidl)
{
	gchar *path, *buf;
	FILE *fd;

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	if (!(fd = fopen (path, "rt")))
	{
		c2_error_set (-errno);
#ifdef USE_DEBUG
		g_warning ("Unable to search for a UIDL: %s\n", c2_error_get ());
		C2_DEBUG (path);
#endif
		g_free (path);
		return;
	}
	g_free (path);

	for (;;)
	{
		if (!(path = c2_fd_get_line (fd)))
			break;

		buf = c2_str_get_word (1, path, ' ');
		g_free (path);
		
		if (c2_streq (buf, uidl))
			break;
		g_free (buf);
		buf = NULL;
	}

	fclose (fd);

	if (!buf)
		return FALSE;

	g_free (buf);

	return TRUE;
}

static void
uidl_add (C2Account *account, const gchar *uidl, time_t date)
{
	gchar *path;
	FILE *fd;

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	if (!(fd = fopen (path, "at")))
	{
		c2_error_set (-errno);
#ifdef USE_DEBUG
		g_warning ("Unable to add an UIDL: %s\n", c2_error_get ());
#endif
		g_free (path);
		return;
	}
	
	fprintf (fd, "%d %s\n", date, uidl);
	fclose (fd);
}

static void
uidl_clean (C2Account *account)
{
	gchar *path;

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	unlink (path);
}

static gint
retrieve (C2POP3 *pop3, C2Account *account, C2Mailbox *inbox, GSList *download_list)
{
	C2Message *message;
	gchar *string;
	gint nth, len, i;
	gint32 length, total_length = 0;
	gchar *tmp;
	FILE *fd;
	GSList *l;
	gboolean getting_header;
	
	for (l = download_list, i = 1; l; l = g_slist_next (l), i++)
	{
		nth = atoi ((gchar*)l->data);
		
		if (pop3->flags & C2_POP3_DO_KEEP_COPY)
		{
		}

		/* Retrieve */
		if (c2_net_object_send (C2_NET_OBJECT (pop3), "RETR %d\r\n", nth) < 0)
			return -1;

		if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
			return -1;

		if (c2_strnne (string, "+OK", 3))
		{
			string = strstr (string, " ");
			if (string)
				string++;
			
			c2_error_object_set_custom (GTK_OBJECT (pop3), string);
			return -1;
		}

		sscanf (string, "+OK %d octets\r\n", &total_length);

		gtk_signal_emit (GTK_OBJECT (pop3), signals[RETRIEVE], i, 0, total_length);

		/* Get a temp name */
		tmp = c2_get_tmp_file ();

		/* Open it */
		if (!(fd = fopen (tmp, "w")))
		{
#ifdef USE_DEBUG
			g_print ("Unable to open tmp file: %s\n", tmp);
#endif
			g_free (tmp);
			return -1;
		}

		getting_header = TRUE;
		for (length = 0;;)
		{
			c2_net_object_read (C2_NET_OBJECT (pop3), &string);

			if (c2_streq (string, ".\r\n"))
				break;
			
			len = strlen (string);
			if (len == 2 && getting_header)
			{
				getting_header = FALSE;
				fprintf (fd, "X-CronosII-Account: %s\n", account->name);
				fprintf (fd, "X-CronosII-State: %c\n\n", C2_MESSAGE_UNREADED);
			}
			
			string[len-2] = '\n';
			fwrite (string, sizeof (gchar), len-1, fd);

			g_free (string);

			length += len;

			gtk_signal_emit (GTK_OBJECT (pop3), signals[RETRIEVE], nth, length, total_length);
		}

		fclose (fd);

		/* Load the mail */
		message = c2_db_message_get_from_file (tmp);
		c2_db_message_add (inbox, message);
		unlink (tmp);

		/* Now that everything is written, delete the mail
		 * or add the UIDL to the db.
		 */
		if (pop3->flags & C2_POP3_DO_KEEP_COPY)
		{
			gchar uidl[UIDL_LENGTH];
			gchar *prompt;

			prompt = g_strdup_printf ("%d %%s", nth);
			sscanf ((gchar*)l->data, prompt, uidl);
			uidl_add (account, uidl, inbox->db->prev->date);
			g_free (prompt);
		} else
		{
			if (c2_net_object_send (C2_NET_OBJECT (pop3), "DELE %d\r\n", nth) < 0)
				return -1;
			
			if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
				return -1;

			if (c2_strnne (string, "+OK", 3))
			{
				string = strstr (string, " ");
				if (string)
					string++;
				
				c2_error_object_set_custom (GTK_OBJECT (pop3), string);
				return -1;
			}
		}

		gtk_object_destroy (GTK_OBJECT (message));
		g_free (tmp);
	}

	return 0;
}

static gint
synchronize_search_uidl_in_list (GSList *uidl_list, const gchar *uidl)
{
	GSList *l;
	gint n, length;
	gchar *ptr;
	gchar fmt[] = " %%s";
	gchar buf[UIDL_LENGTH] = { 0 };

	/* [HACK]
	 * Here we assume that UIDL is in the following fmt:
	 * # uidl
	 *  ^ (just one space)
	 */
	for (l = uidl_list, n = 0; l; l = g_slist_next (l), n++)
	{
		ptr = strstr ((gchar*) l->data, " ")+1;
		length = strlen (ptr);
		strncpy (buf, ptr, length-2);
		buf[length-1] = 0;
		if (c2_streq (buf, uidl))
			return n;
	}
	
	return -1;
}

static gint
synchronize (C2POP3 *pop3, C2Account *account, GSList *uidl_list)
{
	gchar *path, *tmppath, *line, *buf;
	gint length, i, nth;
	FILE *fd, *tmpfd;
	time_t date;

	length = g_slist_length (uidl_list);

	gtk_signal_emit (GTK_OBJECT (pop3), signals[SYNCHRONIZE], 0, length);

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	if (!(fd = fopen (path, "rt")))
	{
		c2_error_object_set (GTK_OBJECT (pop3), -errno);
#ifdef USE_DEBUG
		g_warning ("Unable to synchronize[1]: %s\n", c2_error_object_get (GTK_OBJECT (pop3)));
#endif
		g_free (path);
		return -1;
	}

	tmppath = c2_get_tmp_file ();
	if (!(tmpfd = fopen (tmppath, "wt")))
	{
		c2_error_object_set (GTK_OBJECT (pop3), -errno);
#ifdef USE_DEBUG
		g_warning ("Unable to synchronize[2]: %s\n", c2_error_object_get (GTK_OBJECT (pop3)));
#endif
		g_free (path);
		fclose (fd);
		g_free (tmppath);
		return -1;
	}

	for (i = 1;; i++)
	{
		if (!(line = c2_fd_get_line (fd)))
			break;

		buf = c2_str_get_word (0, line, ' ');
		
		date = atoi (buf);
		g_free (buf);
		
		if (pop3->copies_in_server_life_time < time (NULL)-date)
		{
			GSList *link;
			gchar *string;
			
			/* We have to remove this one,
			 * check if it is still in the
			 * server (in the uidl_list)
			 */
			buf = c2_str_get_word (1, line, ' ');
			if ((nth = synchronize_search_uidl_in_list (uidl_list, buf)) < 0)
				continue;

			link = g_slist_nth (uidl_list, nth);
			if (c2_net_object_send (C2_NET_OBJECT (pop3), "DELE %d\r\n", atoi (link->data)) < 0)
				return -1;
			uidl_list = g_slist_remove_link (uidl_list, link);
			
			if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
				return -1;

			if (c2_strnne (string, "+OK", 3))
			{
				string = strstr (string, " ");
				if (string)
					string++;
				
				c2_error_object_set_custom (GTK_OBJECT (pop3), string);
				return -1;
			}
		} else
		{
save_uidl:
			fprintf (tmpfd, "%s\n", line);
		}

		g_free (line);
		gtk_signal_emit (GTK_OBJECT (pop3), signals[SYNCHRONIZE], i, length);
	}

	fclose (tmpfd);
	fclose (fd);
	
	/* Clear UIDL database */
	uidl_clean (account);
	
	/* Add all pending elements to the UIDL database */
	c2_file_binary_move (tmppath, path);
	g_free (tmppath);
	g_free (path);
}

static void
quit (C2POP3 *pop3)
{
	c2_net_object_send (C2_NET_OBJECT (pop3), "QUIT\r\n");
}
