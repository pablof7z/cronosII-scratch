#include <stdio.h>
#include <libcronosII/utils.h>

int main (int argc, char **argv)
{
	FILE *fd;
	char *line;
	int i;
	int is_clean;
	
	if (argc < 2)
	{
		fprintf (stderr, "Usage: index_is_clean FILE\n"
				 "\n"
				 "This program is a tool of Cronos II for checking if an index file is clean.\n"
				 "When using Cronos II, after Compacting the mailboxes you should get all\n"
				 "mailboxes cleaned up, so if you use this program on the indexes it should\n"
				 "tell you that they are all clean.\n");
		return 1;
	}

	for (i = 1; i < argc; i++)
	{
		if (!(fd = fopen (argv[i], "r")))
		{
			perror (argv[i]);
			continue;
		}

		is_clean = 1;

		for (;;)
		{
			if (!(line = c2_fd_get_line (fd)))
				break;

			if (!line)
				break;
			
			if (*line == 'D')
			{
				is_clean = 0;
				break;
			}

			g_free (line);
		}

		if (is_clean)
			printf ("%s is clean\n", argv[i]);
		else
		{
			printf ("%s is not clean at %d\n", argv[i], ftell (fd)-strlen (line));
			g_free (line);
		}

		fclose (fd);
	}

	return 0;
}
