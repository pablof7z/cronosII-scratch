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

#define DEFAULT_FLAGS C2_POP3_DONT_KEEP_COPY | C2_POP3_DONT_LOSE_PASSWORD

static void
class_init										(C2POP3Class *klass);

static void
init											(C2POP3 *pop3);

static void
destroy											(GtkObject *object);

static void
my_marshal_NONE__INT_INT_INT					(GtkObject *object, GtkSignalFunc func,
												 gpointer func_data, GtkArg * args);

static gint
welcome											(C2POP3 *pop3);

static gint
login											(C2POP3 *pop3);

static gint
status											(C2POP3 *pop3);


static gint
retrieve										(C2Account *account, gint mails);

enum
{
	STATUS,
	RETRIEVE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObject *parent_class = NULL;

/**
 * c2_pop3_new
 * @user: Username.
 * @pass: Password (might be NULL).
 * @host: Hostame.
 * @port: Port.
 * @ssl: Wheter SSL connection should be used or not.
 *
 * This function will create a new C2POP3 with
 * the data you pass it and with some default configuration.
 * 
 * Return Value:
 * The allocated C2POP3 object or %NULL if there was an error.
 **/
C2POP3 *
c2_pop3_new (const gchar *user, const gchar *pass, const gchar *host, gint port, gboolean ssl)
{
	C2POP3 *pop3;
	
	c2_return_val_if_fail (user || host, NULL, C2EDATA);
	
	pop3 = gtk_type_new (C2_TYPE_POP3);

	pop3->user = g_strdup (user);
	pop3->pass = g_strdup (pass);

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

/**
 * c2_pop3_set_wrong_pass_cb
 * @pop3: C2POP3 object.
 * @func: C2POP3GetPass function.
 *
 * This function sets the function that will be called when a wrong
 * password in this C2POP3 object is found.
 * The C2POP3GetPass type function should return the new password
 * or NULL if it wants to cancel the fetching.
 */
void
c2_pop3_set_wrong_pass_cb (C2POP3 *pop3, C2POP3GetPass func)
{
	c2_return_if_fail (pop3, C2EDATA);

	pop3->wrong_pass_cb = func;
}

/**
 * c2_pop3_fetchmail
 * @account: Loaded C2Account object.
 *
 * This function will download
 * messages from the account @account using
 * the POP3 protocol.
 *
 * Return Value:
 * 0 on success or -1.
 **/
gint
c2_pop3_fetchmail (C2Account *account)
{
	C2POP3 *pop3 = account->protocol.pop3;
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

	if ((mails = status (pop3)) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}

	if (retrieve (account, mails) < 0)
	{
		c2_net_object_disconnect_with_error (C2_NET_OBJECT (pop3));
		pthread_mutex_unlock (&pop3->run_lock);
		return -1;
	}
	printf("done with fetching\n");

	c2_net_object_disconnect (C2_NET_OBJECT (pop3));

	/* Unlock the mutex */
	pthread_mutex_unlock (&pop3->run_lock);

	return 0;
}

static gint
welcome (C2POP3 *pop3)
{
	gchar *string 		= NULL;
	gchar *logintokenpos	= NULL;
	gchar *logintoken	= NULL;
	gint  loginsize 	= 0;

	if (c2_net_object_read (C2_NET_OBJECT (pop3), &string) < 0)
		return -1;

	if(pop3->auth_method == C2_POP3_AUTHENTICATION_APOP)
	{
		// They want to use APOP so we need to get the timestamp and
		// domain name from the welcome message

		logintokenpos 	= strstr(string,"<"); 			/* login token start with the first <  */
		if (logintokenpos == NULL) 				/* no < means they don't support APOP */
		{
			c2_error_set_custom ("Server does not support APOP");
			return -1;
		}

		loginsize 	= strlen(logintokenpos)-1;		/* don't want the trailing \n  */
		logintoken 	= (gchar*)g_malloc(sizeof(gchar) * loginsize);
	
		strncpy(logintoken,logintokenpos,strlen(logintokenpos)); /* copy everything from the "<" to the \0 */
		logintoken[loginsize-1] = 0;				 /* overwrite the \n with the NULL terminator */
	
	
		// printf("Welcome token  is \"%s\"\n",logintoken);
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

	gint 	 i 		= 0;
	gboolean logged_in 	= FALSE;

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
		}
		else
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
			string = strstr (string, " ");
			if (string)
				string++;
			
			g_free (pop3->pass);

			/* set pop3->pass equal to NULL just in case there is no callback function */
			pop3->pass = NULL;

			if (pop3->wrong_pass_cb)
				pop3->pass = pop3->wrong_pass_cb (pop3, string);
		} else
			logged_in = TRUE;
	} while (i++ < 3 && !logged_in && pop3->pass);
	
	if (!logged_in)
		return -1;

	return 0;
}

static gint
status (C2POP3 *pop3)
{
	gchar *string;
	gint mails;

	if (c2_net_object_send (C2_NET_OBJECT (pop3), "STAT\r\n") < 0)
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

	sscanf (string, "+OK %d ", &mails);

	printf ("mails: %d\n", mails);
	gtk_signal_emit (GTK_OBJECT (pop3), signals[STATUS], mails);

	return mails;
}

static gint
retrieve (C2Account *account, gint mails)
{
	C2POP3 *pop3 = account->protocol.pop3;
	C2Message *message;
	gchar *string;
	gint i, len;
	gint32 length, total_length = 0;
	gchar *tmp;
	FILE *fd;
	
	for (i = 1; i <= mails; i++)
	{
		if (pop3->flags & C2_POP3_DO_KEEP_COPY)
		{
L			/* TODO */
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

			printf ("%d: %d\n", __LINE__, total_length);
			gtk_signal_emit (GTK_OBJECT (pop3), signals[RETRIEVE], i, length, total_length);
		}

		fclose (fd);

		/* Load the mail */
		if ((message = c2_db_message_get_from_file (tmp)))
		{
			
		} else
			L

		printf("sure.. lets see if it is here..\n");
		g_free (tmp);
	}

	return 0;
}

GtkType
c2_pop3_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info = {
			"C2Pop3",
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
					my_marshal_NONE__INT_INT_INT, GTK_TYPE_NONE, 3,
					GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_INT);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	object_class->destroy = destroy;
}

static void
init (C2POP3 *pop3)
{
	pop3->user = pop3->pass = NULL;
	pop3->flags = DEFAULT_FLAGS;
	pop3->wrong_pass_cb = NULL;
	pthread_mutex_init (&pop3->run_lock, NULL);
}

static void
destroy (GtkObject *object)
{
	C2POP3 *pop3 = C2_POP3 (object);

	if (c2_net_object_is_offline (C2_NET_OBJECT (pop3)))
#ifdef USE_DEBUG
	{
		g_print ("A C2Pop3 object is being freed while a connection is "
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

typedef void (*C2Signal_NONE__INT_INT_INT)		(GtkObject *object, gint arg1, gint arg2, gint arg3,
													gpointer user_data);

static void
my_marshal_NONE__INT_INT_INT (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg * args)
{
	C2Signal_NONE__INT_INT_INT rfunc;
	rfunc = (C2Signal_NONE__INT_INT_INT) func;
	(*rfunc) (object, GTK_VALUE_INT (args[0]), GTK_VALUE_INT (args[1]), GTK_VALUE_INT (args[2]), func_data);
}
