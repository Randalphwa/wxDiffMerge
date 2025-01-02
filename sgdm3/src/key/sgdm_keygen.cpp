// a little throwaway test program to generate a key
// or test a key.
// 
// usage:
//    sgdm_keygen [chDomain]
//    sgdm_keygen -t Z-XXXXX-XXXXX-XXXXX-XXXXX-XXXXX
//
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "key.h"

//////////////////////////////////////////////////////////////////

static int _usage(const char * szMsg)
{
	fprintf(stderr, "Error: %s\n", szMsg);
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "    sgdm_keygen [chDomain]\n");
	fprintf(stderr, "    sgdm_keygen -t <key>\n");
	return 1;
}

static int _genkey(char chDomain)
{
	char * szKey = sgdm_genkey(chDomain);
	bool bCheck = sgdm_checkkey(szKey);

	if (bCheck)
	{
		printf("%s\n", szKey);
		free(szKey);
		return 0;
	}
	else
	{
		fprintf(stderr, "Error: invalid key generated. %s\n", szKey);
		free(szKey);
		return 1;
	}
}

int my_main(int argc, char ** argv)
{
	switch (argc)
	{
	case 0:
	case 1:
		return _genkey('Z');

	case 2:
		if ((argv[1][0] != '-') && (strlen(argv[1]) == 1))
			return _genkey(argv[1][0]);
		else
			return _usage("argv[1] invalid");

	case 3:
		if (strcmp(argv[1], "-t") == 0)
		{
			bool bCheck = sgdm_checkkey(argv[2]);
			if (bCheck)
			{
				printf("%s: valid\n", argv[2]);
				return 0;
			}
			else
			{
				printf("%s: invalid\n", argv[2]);
				return 1;
			}
		}

	default:
		return _usage("too many args");
	}
}

int main(int argc, char ** argv)
{
	int rc = my_main(argc, argv);

	exit(rc);
}


