#include<stdio.h>
#include<string.h>

main()
{
	char * infileName = "input";
	char * outfileName = "output";
	FILE * infile;
	FILE * outfile;
	infile = fopen("input","r");
	outfile = fopen("output","w");

	char checkSum[4] = {0,0,0,0};
	char buffer[4];
	int bytesRead=fread(buffer, 1, 4, infile);
	while (bytesRead==4) {
		int i;
		for (i=0; i<4; i++) {
			checkSum[i] = (checkSum[i])^(buffer[i]);
		}
		size_t ret = fwrite(buffer, 1, 4, outfile);
		bytesRead=fread(buffer, 1, 4, infile);
	}

	if(bytesRead > 0) {
		int index=0;
		while (((int)(buffer[index])!=10) && (index<4)) {
			index++;
		}

		memset(&buffer[bytesRead],0,4-bytesRead);

		size_t ret = fwrite(buffer, 1, 4, outfile);
		int i;
		for (i=0; i<4; i++) {
			checkSum[i] = (checkSum[i])^(buffer[i]);
		}
	}

	fwrite(checkSum,1,4, outfile);

	fclose(infile);
	fclose(outfile);

	return(0);
}
