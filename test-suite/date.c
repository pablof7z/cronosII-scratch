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
