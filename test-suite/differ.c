#include <stdio.h>

int
main (void)
{
	FILE *f1, *f2;
	char r1[2810], r2[2810];
	int i;

	memset (r1, 2810, sizeof (char));
	memset (r2, 2810, sizeof (char));

	f1 = fopen ("cronosII1.png", "r");
	f2 = fopen ("cronosII2.png", "r");

	fread (r1, 2810, sizeof (char), f1);
	fread (r2, 2810, sizeof (char), f2);

	for (i = 0; i < 2810; i++)
	{
		if (r1[i] != r2[i])
			printf ("Diferencia en %d [%c, %c]\n", i, r1[i], r2[i]);
	}

	return 0;
}
