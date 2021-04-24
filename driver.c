#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

int main(int argc, char **argv)
{
	int lflag = 0, aflag = 0, vflag = 0;
	FILE *ifp;
	char *inputfile;
	char c;
	int i;

	if (argc < 2)
	{
		printf("Error : please include the file name");
		return 0;
	}
	else if (argc > 4)
	{
		lflag = 1;
		aflag = 1;
		vflag = 1;
	}
	else if (argc == 3 || argc == 4)
	{
		if (argv[2][1] == 'l')
			lflag = 1;
		else if (argv[2][1] == 'a')
			aflag = 1;
		else
			vflag = 1;

		if (argc == 4)
		{
			if (argv[3][1] == 'l')
				lflag = 1;
			else if (argv[3][1] == 'a')
				aflag = 1;
			else
				vflag = 1;
		}
	}

	if (lex_analyze(argv[1], lflag) == 0)
	{
		printf("\n");
		if (parse(aflag) == 0)
		{
			printf("\n");
			execute(vflag);
		}
	}

	return 0;
}
