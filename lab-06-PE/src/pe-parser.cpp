#include <iostream>
#include <fstream>
#include "PEFile.h"

int main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("To few arguments for parsing", argv[0]);
		return 1;
	}

	FILE * PpeFile;
	PpeFile=fopen(argv[2], "rb");

	if (PpeFile == NULL) {
		printf("Can't open file.\n");
		return 1;
	}

	
	
	PEFile file(argv[2], PpeFile, argv[1]);
	fclose(PpeFile);
	exit(0);
	

	return 0;
}
