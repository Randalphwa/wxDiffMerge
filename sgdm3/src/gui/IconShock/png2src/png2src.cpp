// png2src -- convert PNG file into C++ source code header file.
// the result can be compiled into the sgdm3 executable.
//////////////////////////////////////////////////////////////////
// usage:
//    png2src [<png-files>...]
//////////////////////////////////////////////////////////////////

#pragma warning( disable : 4996 )

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv)
{
	if (argc < 2)
	{
		fprintf(stderr,"Usage: png2src [png/foo.png]\n");
		fprintf(stderr,"Creates: hdr/foo.h\n");
		exit(1);
	}

	for (int k=1; k<argc; k++)
	{
		// given "png/foo.png"

		char * szPNG = argv[k];
		if (strncmp(szPNG,"png/",4) != 0)
		{
			fprintf(stderr,"Error: filename must start with 'png/': %s\n",szPNG);
			exit(1);
		}
		
		int lenPNG = strlen(szPNG);

		// build "hdr/foo.h"

		char * szH = (char *)malloc(lenPNG+10);
		szH[0] = 0;
		strcat(szH,"hdr/");
		strcat(szH,szPNG+4);

		int ndxDot = lenPNG - 4;
		if (strcmp(szH+ndxDot,".png") != 0)
		{
			fprintf(stderr,"Error: bad filename (must end in '.png'): %s\n",szPNG);
			exit(1);
		}
		szH[ndxDot+1] = 0;
		strcat(szH,"h");

		// build variable name "foo"

		char * szVar = (char *)malloc(lenPNG+10);
		strcpy(szVar,szPNG+4);
		szVar[ndxDot-4] = 0;
		
		printf("Converting: png: %s\n",szPNG);
		printf("            hdr: %s\n",szH);
		printf("            var: %s\n",szVar);

		// read PNG file and convert to crude base16 into .h file

		int fdIn = _open(szPNG,_O_RDONLY|_O_BINARY);
		if (fdIn == -1)
		{
			fprintf(stderr,"Error: cannot read: %s\n",szPNG);
			exit(1);
		}
		
		FILE * fileOut = fopen(szH,"wb");
		if (fileOut == NULL)
		{
			fprintf(stderr,"Error: cannot write: %s\n",szH);
			exit(1);
		}

		fprintf(fileOut,"const char * gsz_%s =\n",szVar);

		int col = 0;
		
		char bufIn;
		while (_read(fdIn,&bufIn,1) == 1)
		{
			unsigned char c0 = (bufIn >> 4) & 0x0f;
			unsigned char c1 = (bufIn     ) & 0x0f;

			if (col == 0)
				fprintf(fileOut,"\"");
			
			fprintf(fileOut,"%01x%01x",c0,c1);
			col += 2;

			if (col == 40)
			{
				fprintf(fileOut,"\"\n");
				col = 0;
			}
		}

		if (col > 0)
			fprintf(fileOut,"\"\n");
			
		fprintf(fileOut,";\n");

		fclose(fileOut);

		_close(fdIn);
	}

	exit(0);
}

