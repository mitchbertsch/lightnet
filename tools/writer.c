#include<stdio.h>
#include<string.h>

int main()
{
        char buffer[4];
        FILE *ptr_myfile;
        char packet[] = "This is the packet that will be sent in chunks of 63 bytes across the IR network to the target device. Thanks to the changes that I just made, I am now able to send much larger packets.";
        int packetIndex = 0;
        int count = 1;

        const char unsigned pulse257[4] = {0x01, 0x01, 0x00, 0x00};
        const char unsigned pulse771[4] = {0x03, 0x03, 0x00, 0x00};
        const char unsigned space257[4] = {0x01, 0x01, 0x00, 0x00};
        const char unsigned flag[4] = {0x01, 0x10, 0x00, 0x00};

        int packetSize = sizeof(packet) - 1;
        printf("Packet size = %d\n",packetSize);

        while(packetIndex < packetSize) {

                int subSize = 63;
                if (packetSize - packetIndex < subSize) {       subSize = packetSize - packetIndex;     }
                char subPacket[63];
                memcpy(subPacket,&packet[packetIndex],subSize);

                ptr_myfile=fopen("/dev/lirc0","wb");
                if (!ptr_myfile)
                {
                        printf("Unable to open file!\n");
                        return 1;
                }
                else
                {
                        printf("Was able to open the file.\n");
                }

                // write the terminal flag
                size_t ret = fwrite(flag,1,4,ptr_myfile);

                // write a space
                ret = fwrite(space257,1,4,ptr_myfile);

                int i;
                for (i=0; i<subSize; i++) {
                        int charInt = subPacket[i];
                        int j;
                        for (j=0; j<8; j++) {
                                if (charInt >= (1<<(7-j))) {
                                        // write a pulse 1
                                        ret = fwrite(pulse771,1,4,ptr_myfile);
                                        charInt = charInt - (1<<(7-j));
                                } else {
                                        // write a pulse 0
                                        ret = fwrite(pulse257,1,4,ptr_myfile);
                                }

                                // write a space
                                ret = fwrite(space257,1,4,ptr_myfile);
                        }
                }

                // write another terminal flag
                ret = fwrite(flag,1,4,ptr_myfile);

                printf("End of file\n");

                fclose(ptr_myfile);
                packetIndex = packetIndex + subSize;
        }
        return 0;
}
