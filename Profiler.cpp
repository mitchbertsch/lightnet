#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <iostream>

using namespace std;

int getLength(char lbyte, char mbyte, char rbyte) {
	int left = (int)lbyte;
	int middle = (int)mbyte;
	int right = (int)rbyte;

	middle <<= 8;
	right <<= 16;

	return left + middle + right;
}
int main() {
	string path = "/dev/lirc0";
	int fd = open(path.c_str(), O_RDWR | O_TRUNC);

	while(1) {
		char buff[4];
		read(fd, buff, 4);
		int length = getLength(buff[0], buff[1], buff[2]);
		string type = ((int)buff[3]) ? "pulse" : "space";
		cout << type << "," << length << endl;
	}
}
