#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <ifstream>
#include <ofstream>


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
					//Download packet
				}
			}
			else {
				//The second one must be a pulse
				int pulseTime = getLength(checkPacket[1][0], checkPacket[1][1], checkPacket[1][2]);
				if (pulseTime >= 900 && pulseTime <= 1100) {
					//Download packet
				}
			}
		}
		else {
			//cout << "No data for 5 microseconds" << endl;
		}
		//Read from TUN/TAP pipe, queue, etc.
	}
}
