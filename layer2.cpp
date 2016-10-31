#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <fstream>


using namespace std;

int getLength(char lbyte, char mbyte, char rbyte) {
	int left = (int)lbyte;
	int middle = (int)mbyte;
	int right = (int)rbyte;
	middle <<= 8; //shift the middle byte by 8 bits (since it is the middle byte)
	right <<= 16; //shift the right byte by 16 bits (since it is the right byte)

	return left + middle + right;
}
int main() {
	string LIRCpath = "/dev/lirc0";
	string inputQueue = "./inputQueue";
	string outputQueue = "./outputQueue";

	const char unsigned pulse257[4] = {0x01, 0x01, 0x00, 0x00};
	const char unsigned pulse771[4] = {0x03, 0x03, 0x00, 0x00};
	const char unsigned space257[4] = {0x01, 0x01, 0x00, 0x00};
	const char unsigned flag[4] = {0xe8, 0x03, 0x00, 0x00};


	int IRfd = open(LIRCpath.c_str(), O_RDWR | O_TRUNC);
	struct timeval IRtv; 
	fd_set IRfds;
	while(1) {
		IRtv.tv_sec = 0;
		IRtv.tv_usec = 5;

		FD_ZERO(&IRfds);
		FD_SET(IRfd, &IRfds);

		int IRreturn;
		
		IRreturn = select(IRfd + 1, &IRfds, NULL, NULL, &IRtv);
		if(IRreturn == -1) {
			cerr << "Error selecting IR read" << endl;
		}
		else if(IRreturn) {
			char checkPacket[2][4];
			read(IRfd, checkPacket[0], 4);
			read(IRfd, checkPacket[1], 4);

			if(checkPacket[0][3] == 1) {
				//The first one is a pulse
				int pulseTime = getLength(checkPacket[0][0], checkPacket[0][1], checkPacket[0][2]);
				if (pulseTime >= 900 && pulseTime <= 1100) {
					char packet[256];
					for (int i = 0; i < 256; i++) {
						int byteInt = 0;
						for(int j = 0; j < 8; j++) {
							byteInt *= 2;
							char bit[4];
							read(IRfd, bit, 4); //And this one ought to be a pulse.
							int length = getLength(bit[0], bit[1], bit[2]);
							if(length >= 671 && length <= 871) { //This is a one.
								byteInt++;
							}
							else if(length >= 157 && length <= 357) { //This is a zero
								//Do nothing
							}
							else cout << "ERROR ERROR" << endl;
							read(IRfd, bit, 4); //This should be a space.
						}
						packet[i] = byteInt;
						byteInt = 0;
					}
					ofstream ofs;
					ofs.open(outputQueue, ofstream::out | ofstream::app);
					if(!ofs.fail()) {
						ofs << packet;
						ofs.close();
					}
				}
			}
			else {
				//The second one must be a pulse
				int pulseTime = getLength(checkPacket[1][0], checkPacket[1][1], checkPacket[1][2]);
				if (pulseTime >= 900 && pulseTime <= 1100) {
					char packet[256];
					for (int i = 0; i < 256; i++) {
						int byteInt = 0;
						for(int j = 0; j < 8; j++) {
							byteInt *= 2;
							char bit[4];
							read(IRfd, bit, 4); //This one should be a space.
							read(IRfd, bit, 4); //This one ought to be a pulse.
							int length = getLength(bit[0], bit[1], bit[2]);
							if(length >= 671 && length <= 871) { //This is a one.
								byteInt++;
							}
							else if(length >= 157 && length <= 357) { //This is a zero
								//Do nothing
							}
							else { cout << "ERROR ERROR" << endl; }
						}
						packet[i] = byteInt;
						byteInt = 0;
					}
					ofstream ofs;
					ofs.open(outputQueue, ofstream::out | ofstream::app);
					if(!ofs.fail()) {
						ofs << packet;
						ofs.close();
					}
				}
			}
		}
		else {
			//cout << "No data for 5 microseconds" << endl;
		}
		//Read from TUN/TAP pipe, queue, etc.
		ifstream ifs;
		ifs.open(inputQueue, ifstream::in);
		if(!ifs.fail() && ifs.peek() != ifstream::traits_type::eof()) {
			char writePacket[256];
			for(int i = 0; i < 256; i++) {
				writePacket[i] = ifs.get();
			}
			write(IRfd, flag, 4);
			for(int i = 0; i < 256; i++) {
				int charInt = writePacket[i];
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
			write(IRfd, flag, 4);
			ifs.close();
		}
	}
}
