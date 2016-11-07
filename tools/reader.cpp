#include<stdio.h>
#include<string.h>

int main()
{
        char buffer[4];
        FILE *ptr_myfile;
        char packet[257];
        int packetIndex = 0;

        ptr_myfile=fopen("/dev/lirc0","rb");
        if (!ptr_myfile)
        {
                printf("Unable to open file!\n");
                return 1;
        }


        int byteIndex = 0;
        int byteInt = 0;
        while (fread(buffer, 1, 4, ptr_myfile) == 4) {
                long pulseLength = 256*(256*((int)buffer[2])+((int)buffer[1]))+((int)buffer[0]);
                if((int)buffer[3]) {
                        if (pulseLength > 3000) {
                                packet[packetIndex] = '\0';
                                printf("%s\n",packet);
                                memset(packet,0,sizeof(packet));
                                packetIndex = 0;
                        }
                        else
                        {
                                byteInt*=2;
                                if (pulseLength > 500)
                                {
                                        byteInt++;
                                }
                                if (byteIndex == 7)
                                {
                                        packet[packetIndex] = byteInt;
                                        packetIndex++;
                                        byteIndex = 0;
                                        byteInt = 0;
                                }
                                else
                                {
                                        byteIndex++;
                                }
                        }
                }
                else
                {
                        if (pulseLength > 100000) {
                                printf("Timeout: reset\n");
                                byteInt = 0;
                                byteIndex = 0;
                                memset(packet,0,sizeof(packet));
                                packetIndex = 0;
                        }
                }
        }

        printf("End of file\n");

        fclose(ptr_myfile);
        return 0;
}
