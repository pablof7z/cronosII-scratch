#include <glib.h>
#include <stdlib.h>

#include "error.h"
#include "mailbox.h"
#include "utils.h"

/**
 * c2_mailbox_new
 * @head: List of mailboxes.
 * @name: Name of new mailbox.
 * @parent_id: ID of parent mailbox or -1 if top mailbox.
 * 
 * This function will create a new mailbox
 * which will be child of @parent_id.
 * Note: You probably want to use c2_mailbox_append after
 * this function.
 * 
 * Return Value:
 * The new mailbox.
 **/
C2Mailbox *
c2_mailbox_new (C2Mailbox *head, const gchar *name, gint parent_id) {
	C2Mailbox *mbox;
	
	c2_return_val_if_fail (name, NULL, C2EDATA);

	mbox = g_new0 (C2Mailbox, 1);
	mbox->name = g_strdup (name);
	mbox->id = c2_mailboxes_next_id (head);
	mbox->parent_id = (parent_id < 0) ? mbox->id : parent_id;
	mbox->db = NULL;
	mbox->last_mid = -1;
	mbox->last_row = -1;
	return mbox;
}

/**
 * c2_mailbox_parse
 * @info: Information of the mailbox to parse ($id\r$name\r$parent_id)
 *
 * This function will parse an string (probably
 * readed from the config file) to load it
 * into a valid C2Mailbox.
 *
 * Return Value:
 * The C2Mailbox object.
 **/
C2Mailbox *
c2_mailbox_parse (const gchar *info) {
	C2Mailbox *mbox;
	gchar *buf;

	c2_return_val_if_fail (info, NULL, C2EDATA);

	mbox = g_new0 (C2Mailbox, 1);

	buf = c2_str_get_word (0, info, '\r');
	mbox->id = atoi (buf);
	g_free (buf);

	mbox->name = c2_str_get_word (1, info, '\r');

	buf = c2_str_get_word (2, info, '\r');
	mbox->parent_id = atoi (buf);
	g_free (buf);

	mbox->db = NULL;
	mbox->last_mid = -1;
	mbox->last_row = -1;

	mbox->next = NULL;
	mbox->child = NULL;

	return mbox;
}

/**
 * c2_mailbox_append
 * @head: List of mailboxes (Can be null).
 * @mailbox: Mailbox to add.
 *
 * This function will add @mailbox to the mailboxes @list.
 *
 * Return Value:
 * The list with the new entry.
 **/
C2Mailbox *
c2_mailbox_append (C2Mailbox *head, C2Mailbox *mailbox) {
	C2Mailbox *l;
	
	c2_return_val_if_fail (mailbox, NULL, C2EDATA);

	if (!head)
		return mailbox;

	if (mailbox->id == mailbox->parent_id)
	{
		/* Insert must be done in the top of the tree */
		for (l = head; l->next != NULL; l = l->next);
		l->next = mailbox;
	} else
	{
		C2Mailbox *parent = c2_mailbox_search_id (head, mailbox->parent_id);

		if (parent)
		{
			if (parent->child)
			{
				C2Mailbox *s;
				
				for (s = parent->child; s->next != NULL; s = s->next);
				s->next = mailbox;
			} else
				parent->child = mailbox;
		}
		else
			g_print ("Couldn't append\n");
	}

	return head;
}

/**
 * c2_mailboxes_next_id
 * @head: A pointer to the list of mailboxes (Can be null).
 *
 * This function will get the next available mailbox ID.
 *
 * Return Value:
 * A gint with the next available mailbox ID.
 **/
gint
c2_mailboxes_next_id (C2Mailbox *head) {
	C2Mailbox *l;
	gint next = 0;
	gint tmp;
	
	for (l = head; l != NULL; l = l->next)
	{
		if (l->id >= next) next = l->id+1;
		if (l->child)
		{
			tmp = c2_mailboxes_next_id (l->child);
			if (tmp >= next) next = tmp+1;
		}
	}

	return next;
}

/**
 * c2_mailbox_search_id
 * @head: First node where to start the search (Can be null).
 * @id: ID being search.
 *
 * This function searches recursively for an ID in @head.
 *
 * Return Value:
 * A pointer to the C2Mailbox with ID @id.
 **/
C2Mailbox *
c2_mailbox_search_id (C2Mailbox *head, gint id) {
	C2Mailbox *l, *s;
	gint i = 0;

	for (l = head; l != NULL; l = l->next, i++)
	{
		if (l->id == id)
			return l;
		if (l->child)
			if ((s = c2_mailbox_search_id (l->child, id)) != NULL)
				return s;
	}

	return NULL;
}

/**
 * c2_mailbox_search_name
 * @head: First node where to start the search (Can be null).
 * @name: Mailbox name being searched.
 *
 * This function searches recursively for a @name in @head.
 *
 * Return Value:
 * A pointer to the C2Mailbox with name @name.
 **/
C2Mailbox *
c2_mailbox_search_name (C2Mailbox *head, const gchar *name) {
	C2Mailbox *l, *s;
	gint i = 0;

	for (l = head; l != NULL; l = l->next, i++)
	{
		if (c2_streq (l->name, name))
			return l;
		if (l->child)
			if ((s = c2_mailbox_search_name (l->child, name)) != NULL)
				return s;
	}

	return NULL;
}

void
c2_mailbox_free (C2Mailbox *mbox) {
	c2_return_if_fail (mbox, C2EDATA);

	if (mbox->db)
		c2_db_unload (mbox->db);
	g_free (mbox->name);
	g_free (mbox);
}
