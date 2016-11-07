#include "Lightnet.h"

int LightnetLIRC::getLength(char lbyte, char mbyte, char rbyte) {
  int left = (int)lbyte;
  int middle = (int)mbyte;
  int right = (int)rbyte;
  middle <<= 8; //shift the middle byte by 8 bits (since it is the middle byte)
  right <<= 16; //shift the right byte by 16 bits (since it is the right byte)
  return left + middle + right;
}

int LightnetLIRC::init(string LIRCpath)
{
	path = LIRCpath;
  IRfd = open(LIRCpath.c_str(), O_RDWR | O_TRUNC);
  if(IRfd < 0){
    perror("Opening LIRC interface");
	return IRfd;
  }
  IRtv.tv_sec = 0;
  IRtv.tv_usec = 5;
  FD_ZERO(&IRfds);
  FD_SET(IRfd, &IRfds);
  return 1;
}

void LightnetLIRC::run() {
	cerr << "lirc start\n";
	char subPacket[SUBBUFSIZE];
	while(1) {
		this->init(path);
		int IRreturn;
		
		IRreturn = select(IRfd + 1, &IRfds, NULL, NULL, &IRtv);
		if(IRreturn == -1) {
			cerr << "Error selecting IR read" << endl;
		}
		else if(IRreturn) {
			char checkPacket[2][4];
			read(IRfd, checkPacket[0], 4);
			read(IRfd, checkPacket[1], 4);

			if(checkPacket[1][3] == 1) {
				//The second one is a pulse
				int pulseTime = getLength(checkPacket[0][0], checkPacket[0][1], checkPacket[0][2]);
				if (pulseTime >= 900 && pulseTime <= 1100) {
					vector<char> packet(BUFSIZE);
					char bit[4];
					read(IRfd, bit, 4); //And this one ought to be a space.
					read(IRfd, bit, 4);
					int length = getLength(bit[0], bit[1], bit[2]);
					while (!(length >= 900 && length <= 1100)) {
						int i = 0;
						int byteInt = 0;
						for(int j = 0; j < 8; j++) {
							byteInt *= 2;
							if(length >= 671 && length <= 871) { //This is a one.
								byteInt++;
							}
							else if(length >= 157 && length <= 357) { //This is a zero
								//Do nothing
							}
							else cout << "ERROR ERROR" << endl;
						}
						packet[i] = (char)byteInt;
						byteInt = 0;
						read(IRfd, bit, 4); //And this one ought to be a space.
						read(IRfd, bit, 4); //This one should be a pulse;
						length = getLength(bit[0], bit[1], bit[2]);
						i++;
					}
					lirc_packet ir_tmp;
					ir_tmp.length = packet.size();
					ir_tmp.type = (packet.size() <= 4) ? ACK : DATA;
					memcpy(ir_tmp.buff,packet.data(),ir_tmp.length);
					lnet->push_lirc_rx(ir_tmp);
				}
			}
			else {
				//The second one is not a pulse. This is a problem....
				cerr << "Pulse detection failed" << endl;
			}
		}
		else {
			//cout << "No data for 5 microseconds" << endl;
		}
		//Read from TUN/TAP pipe, queue, etc.
		close(IRfd);
		if(!lnet->empty_lirc_tx()) {
			lirc_packet ir_tmp = lnet->pop_lirc_tx();
			int packetIndex = 0;
			this->init(path);
			write(IRfd, flag, 4);//write flag
			write(IRfd, pulse257, 4);
			close(IRfd);
			while(packetIndex < ir_tmp.length) {
				this->init(path);
				int subSize = SUBBUFSIZE;
				
              			 if (ir_tmp.length - packetIndex < SUBBUFSIZE)
					subSize = SUBBUFSIZE - packetIndex;
					
				for(int i = 0; i < subSize; i++) {
					int charInt = ir_tmp.buff[i+packetIndex];
					for(int j = 0; j < 8; j++) {
						if (charInt >= (1 << (7 - j))) {
							//Write 1.
							write(IRfd, pulse771, 4);
							charInt = charInt - ( 1 << (7 - j));
						}
						else {
							//Write 0;
							write(IRfd, pulse257, 4);
						}
					}
				}
				packetIndex+=subSize;
				close(IRfd);
			}
			this->init(path);
			write(IRfd, flag, 4);
			close(IRfd);
			gettimeofday(&(ir_tmp.sent), NULL); 
			if(ir_tmp.type == DATA)
				lnet->push_lirc_pending(ir_tmp);
		}
	}
}
