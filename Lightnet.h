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
class LightnetTAP;
class LightnetLIRC;

enum PacketType {EMPTY, ETHERNET, LIRCDATA, LIRCACK};//empty not used

/*class MAC
{
  public:
    char address[6];
};*/

class Packet
{
  public:
    int length;
	int priority = 100;
	int transmissions = 0;
    char buff[BUFSIZE];
    PacketType type;
    struct timeval sent;
	bool operator<(const Packet& rhs) const {return (this->priority)*(this->type) < (rhs.priority)*(rhs.type);}
};

class Lightnet
{
  public:
    Lightnet();
	~Lightnet();
	int init(unsigned char addr, string path);
    int init_tap(unsigned char addr);
	int init_lirc(string path);
    const unsigned char ether_mac[5] = {0x6E,0xE2,0xE0,0x7F,0xFA};
	const unsigned char ipv4[3] = {0xC0,0xA8,0x00};
	unsigned int mtu = 1280;
	int timeout = 10;
	int nodes = 5;
	void run();
	int empty_lirc_tx();
	int empty_lirc_rx();
	int empty_lirc_pending();
	int empty_ether_tx();
	int empty_ether_rx();
	Packet pop_lirc_tx();
	Packet pop_lirc_rx();
	Packet pop_lirc_pending();
	Packet pop_ether_tx();
	Packet pop_ether_rx();
	void push_lirc_tx(Packet p);
	void push_lirc_rx(Packet p);
	void push_lirc_pending(Packet p);
	void push_ether_tx(Packet p);
	void push_ether_rx(Packet p);
	vector<LightnetTAP*> taps;
	vector<LightnetLIRC*> lircs;
	int debug_main = 7;
	int crc = 0;
	int transmissions = 1;
	int multithread = 1;
	void ether_to_lirc(Packet& p);
    void lirc_to_ether(Packet& p);
	Packet lirc_ack(Packet& p);
	int check_crc(Packet& p);
	void append_crc(Packet& p);
	void remove_crc(Packet& p);
	//int lirc_dst(Packet& p);
	void remove_pending(Packet& ir_ack);
	void clear_pending();
	int lirc_id(Packet& p);
  private:
    pthread_mutex_t lock_lirc_tx, lock_lirc_rx, lock_lirc_pending, lock_ether_tx, lock_ether_rx;
	pthread_attr_t attr;
    priority_queue<Packet> lirc_tx;
    priority_queue<Packet> lirc_rx;
    vector<Packet> lirc_pending;
    priority_queue<Packet> ether_tx;
    priority_queue<Packet> ether_rx;
	//vector<MAC> addresses;
	pthread_t p_threads[MAXTHREADS];// Threads
	int thread_count = 0;
};


class LightnetTAP
{

  public:
    LightnetTAP(Lightnet* ln) : lnet(ln) {};
    int init(unsigned char addr);
    void run();
	void iteration();
	static void *helper(void *context) {((LightnetTAP *)context)->run();};
	int debug_tap = 0;
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
	const char unsigned pulse_zero[4] = {0x01, 0x01, 0x00, 0x00};
	const char unsigned pulse_one[4] = {0x01, 0x03, 0x00, 0x00};
	const char unsigned pulse_data_flag[4] = {0x01, 0x07, 0x00, 0x00};
	const char unsigned pulse_sub_flag[4] = {0x01, 0x05, 0x00, 0x00};
	const char unsigned pulse_ack_flag[4] = {0x01, 0x09, 0x00, 0x00};
	const char unsigned space[4] = {0x01, 0x01, 0x00, 0x00};
	//const int pulse_zero_min = 127;
	//const int pulse_zero_max = 383;
	const int pulse_one_min = 512;
	const int pulse_one_max = 1024;
	const int pulse_sub_flag_min = 1025;
	const int pulse_sub_flag_max = 1536;
	const int pulse_data_flag_min = 1537;
	const int pulse_data_flag_max = 2048;
	const int pulse_ack_flag_min = 2049;
	const int pulse_ack_flag_max = 2560;
	const int listen = 10000;
	const int gap = 10000;
	int debug_lirc = 6;
  private:
	string path;
	Lightnet* lnet;
    int get_length(char lbyte, char mbyte, char rbyte);
	int open_rd();
	FILE* open_wr();
};
