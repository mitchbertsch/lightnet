#include<stdio.h>
#include<string.h>

main()
{
	char * fileName = "output";
	FILE * fp;
	fp = fopen(fileName,"r");
	
	unsigned long checkSum = 0;
	char buffer[4];
	int bytesRead=fread(buffer, 1, 4, fp);
	while (bytesRead==4) {
		unsigned long bufNum = 256*(256*(256*((int)buffer[0])+((int)buffer[1]))+((int)buffer[2]))+((int)buffer[3]);
		checkSum = checkSum^bufNum;
		bytesRead=fread(buffer, 1, 4, fp);
	}
	
	if(checkSum == 0) {
		printf("Confirmed\n\r");
	} else {
		printf("Invalid\n\r");
	}
}
