#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
int main()
{
        char buffer[4];
        int fd;
        char packet[257];
        int packetIndex = 0;
		FILE *ptr_myfile;
		char tpacket[] = "This is the packet that will be sent in chunks of 63 bytes across the IR network to the target device. Thanks to the changes that I just made, I am now able to send much larger packets.";
		
		const char unsigned pulse257[4] = {0x01, 0x01, 0x00, 0x00};
        const char unsigned pulse771[4] = {0x03, 0x03, 0x00, 0x00};
        const char unsigned space257[4] = {0x01, 0x01, 0x00, 0x00};
        const char unsigned flag[4] = {0x01, 0x10, 0x00, 0x00};
		
		int packetSize = sizeof(tpacket) - 1;
        printf("Packet size = %d\n",packetSize);

	while(1)
	{
	
        fd=open("/dev/lirc0",O_RDWR | O_TRUNC | O_NONBLOCK);
        if (!fd)
        {
                printf("Unable to open file!\n");
                return 1;
        }


		int byteIndex = 0;
        int byteInt = 0;
		int cycles = 10000000;//10s
		while(cycles > 0){
		int tmp = read(fd, buffer, 4); 
				if(tmp == 4) {
					long pulseLength = 256*(256*((int)buffer[2])+((int)buffer[1]))+((int)buffer[0]);
					if((int)buffer[3]) {
							if (pulseLength > 3000) {
									packet[packetIndex] = '\0';
									printf("%s - %i\n",packet,sizeof(packet));
									printf("%i\n",cycles);
									memset(packet,0,sizeof(packet));
									packetIndex = 0;
									cycles = 500000;//500ms
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

			}else
				cycles--;
	//
		}
        printf("End of file\n");
        close(fd);
		usleep(100000);
		
		

		{
			packetIndex = 0;
			 while(packetIndex < packetSize) {

                int subSize = 63;
                if (packetSize - packetIndex < subSize) {       subSize = packetSize - packetIndex;     }
                char subPacket[63];
                memcpy(subPacket,&tpacket[packetIndex],subSize);

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
		
		
		
		}
		
	}
        return 0;
}
