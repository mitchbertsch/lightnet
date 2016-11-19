#include "Lightnet.h"

int LightnetLIRC::get_length(char lbyte, char mbyte, char rbyte) {
  int left = (int)lbyte;
  int middle = (int)mbyte;
  int right = (int)rbyte;
  middle <<= 8; //shift the middle byte by 8 bits (since it is the middle byte)
  right <<= 16; //shift the right byte by 16 bits (since it is the right byte)
  return left + middle + right;
}

int LightnetLIRC::open_rd()
{
  int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
  if(fd < 0)
    perror("Opening LIRC interface");
  return fd;
}

FILE* LightnetLIRC::open_wr()
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
	
	if(debug_lirc>6)
		cerr << "read mode\n";
	fd = open_rd();
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
			pulseLength = get_length(bit[0], bit[1], bit[2]);
			if((int)bit[3]) {
				if (pulseLength >= pulse_ack_flag_min && pulseLength <= pulse_ack_flag_max) {
					if(packetIndex != 0)
					{
						Packet ir_tmp;
						ir_tmp.length = packetIndex;
						ir_tmp.type = LIRCACK;
						memcpy(ir_tmp.buff,packet,ir_tmp.length);
						lnet->push_lirc_rx(ir_tmp);
						rcycles = (rand() % MAXNODES + 1);
						memset(packet,0,BUFSIZE);
						packetIndex = 0;
						byteInt = 0;
						byteIndex = 0;
					}
					if(debug_lirc>6)
						cerr << "ackflag\n";
				}else if (pulseLength >= pulse_data_flag_min && pulseLength <= pulse_data_flag_max) {
					if(packetIndex != 0)
					{
						Packet ir_tmp;
						ir_tmp.length = packetIndex;
						ir_tmp.type = LIRCDATA;
						memcpy(ir_tmp.buff,packet,ir_tmp.length);
						lnet->push_lirc_rx(ir_tmp);
						rcycles = (rand() % MAXNODES + 1);
						memset(packet,0,BUFSIZE);
						packetIndex = 0;
						byteInt = 0;
						byteIndex = 0;
					}
					if(debug_lirc>6)
						cerr << "dataflag\n";
				}else if (pulseLength >= pulse_sub_flag_min && pulseLength <= pulse_sub_flag_max) {
					if(debug_lirc)
						cerr << "subflag\n";
				}else{
					byteInt*=2;
					if (pulseLength >= pulse_one_min && pulseLength <= pulse_one_max)
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
					if(debug_lirc>7)
						cerr << "byte\n";
				}
				cycles = rcycles;
			}
			else
			{
				if (pulseLength > gap) {
					if(debug_lirc>6)
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
    if(debug_lirc>6)
		cerr << "write mode\n";
	//usleep(units);
	
					
		
	if(!lnet->empty_lirc_tx()) {
	
		file=open_wr();
		packetIndex = 0;
		Packet ir_tmp = lnet->pop_lirc_tx();
		while(packetIndex < ir_tmp.length) {
			int subSize = SUBBUFSIZE;
			if (ir_tmp.length - packetIndex < SUBBUFSIZE)
				subSize = ir_tmp.length - packetIndex;
				
			// write the flag
			if(ir_tmp.type == LIRCACK)
				fwrite(pulse_ack_flag,1,4,file);
			else if(packetIndex == 0)
				fwrite(pulse_data_flag,1,4,file);
			else
               	fwrite(pulse_sub_flag,1,4,file);
			
			// write a space
            fwrite(space,1,4,file);
			
			for (int i=0; i<subSize; i++) {
                int charInt = ir_tmp.buff[i+packetIndex];
                int j;
                for (int j=0; j<8; j++) {
                    if (charInt >= (1<<(7-j))) {
                           // write a pulse 1
                            fwrite(pulse_one,1,4,file);
                            charInt = charInt - (1<<(7-j));
                    } else {
                            // write a pulse 0
                            fwrite(pulse_zero,1,4,file);
                    }
						// write a space
                        fwrite(space,1,4,file);
                    }
                }

			packetIndex = packetIndex + subSize;
			cerr << "pi: " << packetIndex << "/" << ir_tmp.length << endl;
               // write another terminal flag
			if(ir_tmp.type == LIRCACK)
				fwrite(pulse_ack_flag,1,4,file);
			else if(packetIndex == ir_tmp.length)
				fwrite(pulse_data_flag,1,4,file);
			else
               	fwrite(pulse_sub_flag,1,4,file);
               fflush(file);
		}
		fclose(file);
		if(ir_tmp.type == LIRCDATA)
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
