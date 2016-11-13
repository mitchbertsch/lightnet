#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>
#include <queue>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

/* buffer for reading from tun/tap interface, must be >= 1500 */
#define BUFSIZE 2000
#define SUBBUFSIZE 63
#define MAXTHREADS 4 
#define MAXNODES 254

class Lightnet;
class LightnetTap;
class LightnetLIRC;

enum packet_type {EMPTY, DATA, ACK};//empty not used

class ether_packet
{
  public:
    int length;
	int priority = 100;
    char buff[BUFSIZE];
	bool operator<(const ether_packet& rhs) const {return this->priority < rhs.priority;}
};

class lirc_packet
{
  public:
    int length;
	int priority = 100;
    char buff[BUFSIZE];
    packet_type type;
    struct timeval sent;
	bool operator<(const lirc_packet& rhs) const {return (this->priority)*(this->type) < (rhs.priority)*(rhs.type);}
};

class Lightnet
{
  public:
    Lightnet();
	~Lightnet();
	int init(unsigned char addr, string path);
    int init_tap(unsigned char addr);
	int init_lirc(string path);
	const unsigned char ether_flag[4] = {0x00,0x00,0x10,0x0D};
    const unsigned char ether_mac[5] = {0x6E,0xE2,0xE0,0x7F,0xFA};
	const unsigned char ipv4[2] = {0xC0,0xA8};;
	unsigned int mtu = 576;
	double timeout = 5;
	void run();
	int empty_lirc_tx();
	int empty_lirc_rx();
	int empty_lirc_pending();
	int empty_ether_tx();
	int empty_ether_rx();
	lirc_packet pop_lirc_tx();
	lirc_packet pop_lirc_rx();
	lirc_packet pop_lirc_pending();
	ether_packet pop_ether_tx();
	ether_packet pop_ether_rx();
	void push_lirc_tx(lirc_packet p);
	void push_lirc_rx(lirc_packet p);
	void push_lirc_pending(lirc_packet p);
	void push_ether_tx(ether_packet p);
	void push_ether_rx(ether_packet p);
	vector<LightnetTap*> taps;
	vector<LightnetLIRC*> lircs;
	int debugMain = 1;
	
	lirc_packet ether_to_lirc(ether_packet& erp);
    ether_packet lirc_to_ether(lirc_packet& irp);
	lirc_packet ether_ack(ether_packet& erp);
	int ether_crc(ether_packet& erp);
	int ir_dst(lirc_packet& irp);
	
  private:
    pthread_mutex_t lock_lirc_tx, lock_lirc_rx, lock_lirc_pending, lock_ether_tx, lock_ether_rx;
	pthread_attr_t attr;
    priority_queue<lirc_packet> lirc_tx;
    priority_queue<lirc_packet> lirc_rx;
    priority_queue<lirc_packet> lirc_pending;
    priority_queue<ether_packet> ether_tx;
    priority_queue<ether_packet> ether_rx;
	vector<unsigned char> addresses;
	pthread_t p_threads[MAXTHREADS];// Threads
	int thread_count = 0;
};


class LightnetTap
{

  public:
    LightnetTap(Lightnet* ln) : lnet(ln) {};
    int init(unsigned char addr);
    void run();
	void iteration();
	static void *helper(void *context) {((LightnetTap *)context)->run();};
	int debugTap = 0;
  private:
    int tun_alloc(char *dev, int flags);
    int cread(int fd, char *buf, int n);
    int cwrite(int fd, char *buf, int n);
    int tap_fd;
	int nread, nwrite;
    char buffer[BUFSIZE];
	Lightnet* lnet;

};

class LightnetLIRC
{

  public:
    LightnetLIRC(Lightnet* ln) : lnet(ln) {};
    int init(string path);
    void run();
	void iteration();
	static void *helper(void *context) {((LightnetLIRC *)context)->run();};
	const char unsigned pulseZero[4] = {0x01, 0x01, 0x00, 0x00};
	const char unsigned pulseOne[4] = {0x01, 0x02, 0x00, 0x00};
	const char unsigned pulseDataFlag[4] = {0x01, 0x04, 0x00, 0x00};
	const char unsigned pulseSubFlag[4] = {0x01, 0x03, 0x00, 0x00};
	const char unsigned pulseAckFlag[4] = {0x01, 0x05, 0x00, 0x00};
	const char unsigned space[4] = {0x01, 0x01, 0x00, 0x00};
	const int pulseZeroMin = 127;
	const int pulseZeroMax = 383;
	const int pulseOneMin = 385;
	const int pulseOneMax = 639;
	const int pulseSubFlagMin = 641;
	const int pulseSubFlagMax = 895;
	const int pulseDataFlagMin = 897;
	const int pulseDataFlagMax = 1151;
	const int pulseAckFlagMin = 1153;
	const int pulseAckFlagMax = 1407;
	const int listen = 10000;
	const int gap = 10000;
	int debugLIRC = 6;
  private:
	string path;
	Lightnet* lnet;
    int getLength(char lbyte, char mbyte, char rbyte);
	int openRD();
	FILE* openWR();
};
