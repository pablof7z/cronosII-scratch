/*
 * The db module will handle the database request.
 * All interactions with the database of Cronos II should be done through
 * this API.
 * You might want to compare this module with the old Message module.
 */
#ifndef DB_H
#define DB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

typedef gint mid_t;

typedef enum
{
	C2_DB_NODE_UNREAD,
	C2_DB_NODE_REPLIED,
	C2_DB_NODE_FORWARDED,
	C2_DB_NODE_READED
} C2DBNodeStatus;

typedef struct
{
	/* Position in the database */
	gint row;
	mid_t mid;

	/* Headers of node (Subject, From, Date, Account) */
	gchar *headers[4];
	
	C2DBNodeStatus status;
	int marked : 1;
} C2DBNode;

typedef struct
{
	gchar *mbox;
	GList *head;
} C2DB;

typedef struct
{
	gchar *mbox;
	mid_t mid;
	gchar *message;
	gchar *header;
	gchar *body;
	GList *mime;
} C2DBMessage;

#define c2_db_new()						(g_new0 (C2DB, 1))
#define c2_db_node_new()				(g_new0 (C2DBNode, 1))

C2DB *
c2_db_load									(const gchar *db_name);

void
c2_db_unload								(C2DB *db_d);

gint
c2_db_message_add							(C2DB **db_d, const gchar *message, gint row);

gint
c2_db_message_remove						(C2DB **db_d, int row);

C2DBMessage *
c2_db_message_get							(C2DB *db_d, int row);

C2DBMessage *
c2_db_message_get_from_file			(const gchar *filename);

gint
c2_db_message_search_by_mid			(const C2DB *db_d, mid_t mid);

#ifdef __cplusplus
}
#endif

#endif
