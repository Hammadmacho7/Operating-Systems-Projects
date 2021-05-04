

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
	FILE *fp;
	int x;
	int count, i;
	int maxnum = 100000;

	if (argc != 4) {
		printf("usage: gen <outfile> <numofintegers> <maxnum> \n");
		exit(1);
	}

	count = atoi(argv[2]);
	maxnum = atoi(argv[3]);

	fp = fopen(argv[1], "w");

	
	srand(100);
	for (i = 1; i <= count; ++i) {
		x = ((double) rand() / RAND_MAX) * maxnum;
		fprintf(fp, "%d\n", x);
	}
   



	fclose(fp);
	exit(1);
}
