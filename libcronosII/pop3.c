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

#include "error.h"
#include "net-object.h"
#include "pop3.h"
#include "utils.h"
#include "md5.h"

#define DEFAULT_FLAGS C2_POP3_DO_NOT_KEEP_COPY | C2_POP3_DO_NOT_LOSE_PASSWORD

static void
class_init										(C2POP3Class *klass);

static void
init											(C2POP3 *pop3);

static void
destroy											(GtkObject *object);

static gint
welcome											(C2POP3 *pop3);

static gint
login											(C2POP3 *pop3);

static GSList *
status											(C2POP3 *pop3, C2Account *account);

static gboolean
uidl											(C2POP3 *pop3, C2Account *account, gint mails);

static void
uidl_clean										(C2Account *account);


static gint
retrieve										(C2POP3 *pop3, C2Account *account,
												 C2Mailbox *inbox, GSList *download_list);

enum
{
	LOGIN_FAILED,
	STATUS,
	RETRIEVE,
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

	signals[LOGIN_FAILED] =
		gtk_signal_new ("login_failed",
					GTK_RUN_LAST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2POP3Class, login_failed),
					c2_marshal_INT__POINTER_POINTER_POINTER, GTK_TYPE_INT, 3,
					GTK_TYPE_STRING, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
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
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	klass->login_failed = NULL;
	klass->status = NULL;
	klass->retrieve = NULL;
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
	GSList *download_list;
	gint mails;

	c2_return_val_if_fail (pop3, -1, C2EDATA);

	/* Lock the mutex */
	pthread_mutex_lock (&pop3->run_lock);

	if (c2_net_object_run (C2_NET_OBJECT (pop3)) < 0)
	{
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if (welcome (pop3) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if (login (pop3) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if ((download_list = status (pop3, account)) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if (retrieve (pop3, account, inbox, download_list) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}
	printf("done with fetching\n");

	c2_net_object_disconnect (C2_NET_OBJECT (pop3));
	g_slist_free (download_list);

	/* Unlock the mutex */
	pthread_mutex_unlock (&pop3->run_lock);

	return 0;
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

	if(pop3->auth_method == C2_POP3_AUTHENTICATION_APOP)
	{
		// They want to use APOP so we need to get the timestamp and
		// domain name from the welcome message

		logintokenpos = strstr(string,"<"); 	/* login token start with the first <  */
		if (logintokenpos == NULL) 				/* no < means they don't support APOP */
		{
			c2_error_set_custom ("Server does not support APOP");
			return -1;
		}

		loginsize = strlen(logintokenpos)-1;		/* don't want the trailing \n  */
		logintoken = g_new (gchar, loginsize);
	
		strncpy(logintoken,logintokenpos,strlen(logintokenpos)); /* copy everything from the "<" to the \0 */
		logintoken[loginsize-1] = 0;				 /* overwrite the \n with the NULL terminator */
	
	
		printf("Welcome token is \"%s\"\n",logintoken);
		pop3->logintoken = (gchar*)g_malloc(sizeof(gchar) * loginsize);
		strncpy(pop3->logintoken,logintoken,loginsize); 	/* set the login token so we can use it later in the login call */
		//printf("pop3->logintoken in login is \"%s\"\n",pop3->logintoken);
	}
	else
	{
		pop3->logintoken = NULL;
	}

	if (c2_strnne (string, "+OK", 3))
	{

		string = strstr (string, " ");
		if (string)
			string++;

		c2_error_set_custom (string);
		return -1;
	}
	
	return 0;
}

static gint
login (C2POP3 *pop3)
{
	gchar *string;
	gchar *apopstring;

	unsigned char md5apop[16];
	gchar md5apopstring[33];
	int x;

	gint i = 0;
	gboolean logged_in = FALSE;
	
	if (pop3->auth_method == C2_POP3_AUTHENTICATION_APOP)
	{
		if (pop3->logintoken == NULL)
		{
			/* how the hell did this happen? */
			c2_error_set_custom("Using APOP but didn't get a logintoken in welcome");
			return -1;
		}
		
		// allocate a string for the pass+logintoken so we can get the md5 hash of it
		apopstring = (gchar*)g_malloc(sizeof(gchar) * (strlen(pop3->pass) + strlen(pop3->logintoken) ) + 1);

		strncpy(apopstring,pop3->logintoken,strlen(pop3->logintoken)+1);
		//printf("apopstring is \"%s\" before strcat\n",apopstring);

		strcat(apopstring,pop3->pass);

		md5_buffer(apopstring,strlen(apopstring),md5apop);
		//printf("apopstring is \"%s\" after strcat\n",apopstring);

		// print out the md5 hash
		for( x = 0; x < 16; x++)
		{
			sprintf(&md5apopstring[x*2],"%02x",md5apop[x]);
		}
		md5apopstring[32] = 0;

		//printf("%s\n",md5apopstring);

		if (c2_net_object_send (C2_NET_OBJECT (pop3), "APOP %s %s\r\n", pop3->user,md5apopstring) < 0)
		{
			c2_error_set_custom("Sending APOP login failed");

			return -1;
		}

		if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		{
			c2_error_set_custom("Failed to recv APOP login reply");
			return -1;
		}

		if (c2_strnne (string, "+OK", 3))
		{
			string = strstr (string, " ");
			if (string)
				string++;
	
			c2_error_set_custom (string);
			return -1;
		} else
		{
			return 0;
		}

	}

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

		c2_error_set_custom (string);
		return -1;
	}

	/* Password */
	do
	{
		/* FIXME This crashes when the password is wrong and
		 * is executed for 2 time (probably for >1 time) see NULL setting below. */
		g_free (string);
		if (c2_net_object_send (C2_NET_OBJECT (pop3), "PASS %s\r\n", pop3->pass) < 0)
			return -1;
		
		if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
			return -1;

		if (c2_strnne (string, "+OK", 3))
		{
			gchar *newuser, *newpass;
			gboolean ret;

			string = strstr (string, " ");
			if (string)
				string++;
			
			/* set pop3->pass equal to NULL just in case there is no callback function */
			newuser = NULL;
			newpass = NULL;

			gtk_signal_emit (GTK_OBJECT (pop3), signals[LOGIN_FAILED], string, &newuser, &newpass, &ret);

			if (!ret)
				return -1;
			
			if (newpass)
			{
				g_free (pop3->pass);
				pop3->pass = newpass;
			}
		} else
			logged_in = TRUE;
	} while (i++ < 3 && !logged_in && pop3->pass);
	
	if (!logged_in)
		return -1;

	return 0;
}

static GSList *
status (C2POP3 *pop3, C2Account *account)
{
	gchar *string;
	gint mails;
	GSList *list = NULL;

	if (c2_net_object_send (C2_NET_OBJECT (pop3), "STAT\r\n") < 0)
		return NULL;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return NULL;

	if (c2_strnne (string, "+OK", 3))
	{
		string = strstr (string, " ");
		if (string)
			string++;

		c2_error_set_custom (string);
		return NULL;
	}

	sscanf (string, "+OK %d ", &mails);

	/* Bosko's suggestion. 08/09/01 --pablo */
	if (!mails)
	{
		C2_DEBUG (account->name);
		uidl_clean (account);
	}
	
	if (pop3->flags & C2_POP3_DO_KEEP_COPY)
	{
		gint i;
		
		if (c2_net_object_send (C2_NET_OBJECT (pop3), "UIDL\r\n") < 0)
			return NULL;

		if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
			return NULL;

		if (c2_strnne (string, "+OK", 3))
		{
			g_free (string);
			goto no_uidl;
		}

		for (i = 1; ; i++)
		{
			gchar *prompt;
			gchar *uidl;
			
			if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
				return NULL;

			prompt = g_strdup_printf ("%d %%s\r\n", i);
			C2_DEBUG (prompt);
			sscanf (string, prompt, &uidl);
			C2_DEBUG (uidl);
			g_free (uidl);
			g_free (prompt);
		}
	} else
	{
		/* Append every message to the download list */
		gint i;

no_uidl:
		for (i = 1; i <= mails; i++)
			list = g_slist_append (list, (gpointer) i);
	}

	gtk_signal_emit (GTK_OBJECT (pop3), signals[STATUS], mails);

	return list;
}

static gboolean
uidl_get (C2POP3 *pop3, C2Account *account, const gchar *string)
{
	gchar *path;
	FILE *fd;

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	if (!(fd = fopen (path, "at")))
	{
		c2_error_set (-errno);
#ifdef USE_DEBUG
		g_warning ("Unable to add an UIDL: %s\n", c2_error_get (c2_errno));
#endif
		g_free (path);
		return;
	}
	
	fprintf (fd, "%s\n", string);
	fclose (fd);
}

static void
uidl_set (C2POP3 *pop3, C2Account *account, const gchar *string)
{
	gchar *path;
	FILE *fd;

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	if (!(fd = fopen (path, "at")))
	{
		c2_error_set (-errno);
#ifdef USE_DEBUG
		g_warning ("Unable to add an UIDL: %s\n", c2_error_get (c2_errno));
#endif
		g_free (path);
		return;
	}
	
	fprintf (fd, "%s\n", string);
	fclose (fd);
}

static void
uidl_clean (C2Account *account)
{
	gchar *path;

	path = g_strconcat (g_get_home_dir (), C2_HOME, "uidl" G_DIR_SEPARATOR_S, account->name, NULL);
	printf ("Cleaning UIDL database for %s in '%s'\n", account->name, path);
	unlink (path);
}

static gint
retrieve (C2POP3 *pop3, C2Account *account, C2Mailbox *inbox, GSList *download_list)
{
	C2Message *message;
	gchar *string;
	gint i, len;
	gint32 length, total_length = 0;
	gchar *tmp;
	FILE *fd;
	GSList *l;
	
	for (l = download_list; l; l = g_slist_next (l))
	{
		if (pop3->flags & C2_POP3_DO_KEEP_COPY)
		{
		}

		/* Retrieve */
		if (c2_net_object_send (C2_NET_OBJECT (pop3), "RETR %d\r\n", i) < 0)
			return -1;

		if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
			return -1;

		if (c2_strnne (string, "+OK", 3))
		{
			string = strstr (string, " ");
			if (string)
				string++;
			
			c2_error_set_custom (string);
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

		for (length = 0;;)
		{
			c2_net_object_read (C2_NET_OBJECT (pop3), &string);

			if (c2_streq (string, ".\r\n"))
				break;
			
			len = strlen (string);
			string[len-2] = '\n';
			fwrite (string, sizeof (gchar), len-1, fd);
			g_free (string);

			length += len;

			gtk_signal_emit (GTK_OBJECT (pop3), signals[RETRIEVE], i, length, total_length);
		}

		fclose (fd);

		/* Load the mail */
		if ((message = c2_db_message_get_from_file (tmp)))
		{
			c2_db_message_add (inbox, message);
		} else
			L

		gtk_object_destroy (GTK_OBJECT (message));
		g_free (tmp);
	}

	return 0;
}
