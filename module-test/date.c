/*  Cronos II Mail Client
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
#include <time.h>

#include <libmodules/date-utils.h>

int
main (int argc, char **argv)
{
	time_t t;
	struct tm *tm;
	gchar str[80];
	
	if (argc < 2)
	{
		g_print ("Usage: %s [DATE]\n", argv[0]);
		return 1;
	}

	if ((t = c2_date_parse (argv[1])) < 0)
		t = c2_date_parse_fmt2 (argv[1]);
	tm = localtime (&t);
	strftime (str, sizeof (str), "%Y.%m.%d %H:%M:%S %Z", tm);
	g_print ("%s\n", str);
	return 0;
}
