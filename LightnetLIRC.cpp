#include "Lightnet.h"

int LightnetLIRC::getLength(char lbyte, char mbyte, char rbyte) {
  int left = (int)lbyte;
  int middle = (int)mbyte;
  int right = (int)rbyte;
  middle <<= 8; //shift the middle byte by 8 bits (since it is the middle byte)
  right <<= 16; //shift the right byte by 16 bits (since it is the right byte)
  return left + middle + right;
}

int LightnetLIRC::openRD()
{
  int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
  if(fd < 0)
    perror("Opening LIRC interface");
  return fd;
}

FILE* LightnetLIRC::openWR()
{
  FILE *ptr = fopen("/dev/lirc0","wb");
  if(!ptr)
    perror("Opening LIRC interface");
  return ptr;
}

int LightnetLIRC::init(string LIRCpath)
{
  path = LIRCpath;
  int fd = open(LIRCpath.c_str(), O_RDWR | O_NONBLOCK);
  if(fd < 0){
    perror("Opening LIRC interface");
	return fd;
  }
  close(fd);
  return 1;
}

void LightnetLIRC::iteration()
{
	char bit[4];
	char packet[BUFSIZE];
	FILE* file;
	int fd, packetIndex, byteIndex, byteInt, rcycles, cycles, nread, pulseLength;
	
	if(debugLIRC>6)
		cerr << "read mode\n";
	fd = openRD();
	packetIndex = 0;
	byteIndex = 0;
	byteInt = 0;
	rcycles = (rand() % MAXNODES + 1);//random listen
	memset(packet,0,BUFSIZE);
	cycles = rcycles;
	nread = 0;
	pulseLength = 0;
	while(cycles > 0){
		nread = read(fd, bit, 4);
		if(nread == 4) {
			pulseLength = getLength(bit[0], bit[1], bit[2]);
			if((int)bit[3]) {
				if (pulseLength >= pulseAckFlagMin && pulseLength <= pulseAckFlagMax) {
					if(packetIndex != 0)
					{
						lirc_packet ir_tmp;
						ir_tmp.length = packetIndex;
						ir_tmp.type = ACK;
						memcpy(ir_tmp.buff,packet,ir_tmp.length);
						lnet->push_lirc_rx(ir_tmp);
						rcycles = (rand() % MAXNODES + 1);
						memset(packet,0,BUFSIZE);
						packetIndex = 0;
						byteInt = 0;
						byteIndex = 0;
					}
					if(debugLIRC>6)
						cerr << "ackflag\n";
				}else if (pulseLength >= pulseDataFlagMin && pulseLength <= pulseDataFlagMax) {
					if(packetIndex != 0)
					{
						lirc_packet ir_tmp;
						ir_tmp.length = packetIndex;
						ir_tmp.type = DATA;
						memcpy(ir_tmp.buff,packet,ir_tmp.length);
						lnet->push_lirc_rx(ir_tmp);
						rcycles = (rand() % MAXNODES + 1);
						memset(packet,0,BUFSIZE);
						packetIndex = 0;
						byteInt = 0;
						byteIndex = 0;
					}
					if(debugLIRC>6)
						cerr << "dataflag\n";
				}else if (pulseLength >= pulseSubFlagMin && pulseLength <= pulseSubFlagMax) {
					if(debugLIRC)
						cerr << "subflag\n";
				}else{
					byteInt*=2;
					if (pulseLength >= pulseOneMin && pulseLength <= pulseOneMax)
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
					if(debugLIRC>7)
						cerr << "byte\n";
				}
				cycles = rcycles;
			}
			else
			{
				if (pulseLength > gap) {
					if(debugLIRC>6)
						cerr << "reset\n";
					rcycles = (rand() % MAXNODES + 1);
					memset(packet,0,BUFSIZE);
					packetIndex = 0;
					byteInt = 0;
					byteIndex = 0;
				}
			}
		}else
		{
			usleep(listen);
			cycles--;
		}
	}
	close(fd);
    if(debugLIRC>6)
		cerr << "write mode\n";
	//usleep(units);
	
					
		
	if(!lnet->empty_lirc_tx()) {
	
		file=openWR();
		packetIndex = 0;
		lirc_packet ir_tmp = lnet->pop_lirc_tx();
		while(packetIndex < ir_tmp.length) {
			int subSize = SUBBUFSIZE;
			if (ir_tmp.length - packetIndex < SUBBUFSIZE)
				subSize = ir_tmp.length - packetIndex;
				
			// write the flag
			if(ir_tmp.type == ACK)
				fwrite(pulseAckFlag,1,4,file);
			else if(packetIndex == 0)
				fwrite(pulseDataFlag,1,4,file);
			else
               	fwrite(pulseSubFlag,1,4,file);
			
			// write a space
            fwrite(space,1,4,file);
			
			for (int i=0; i<subSize; i++) {
                int charInt = ir_tmp.buff[i+packetIndex];
                int j;
                for (int j=0; j<8; j++) {
                    if (charInt >= (1<<(7-j))) {
                           // write a pulse 1
                            fwrite(pulseOne,1,4,file);
                            charInt = charInt - (1<<(7-j));
                    } else {
                            // write a pulse 0
                            fwrite(pulseZero,1,4,file);
                    }
						// write a space
                        fwrite(space,1,4,file);
                    }
                }

			packetIndex = packetIndex + subSize;
			cerr << "pi: " << packetIndex << "/" << ir_tmp.length << endl;
               // write another terminal flag
			if(ir_tmp.type == ACK)
				fwrite(pulseAckFlag,1,4,file);
			else if(packetIndex == ir_tmp.length)
				fwrite(pulseDataFlag,1,4,file);
			else
               	fwrite(pulseSubFlag,1,4,file);
               fflush(file);
		}
		fclose(file);
		if(ir_tmp.type == DATA)
		{
			gettimeofday(&(ir_tmp.sent), NULL); 
			ir_tmp.transmissions++;
			lnet->push_lirc_pending(ir_tmp);
		}
	}

}

void LightnetLIRC::run() {
	cerr << "lirc start\n";
	while(1)
		iteration();
}
