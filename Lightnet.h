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
#define MAX_THREADS 4 

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
  private:
    pthread_mutex_t lock_lirc_tx, lock_lirc_rx, lock_lirc_pending, lock_ether_tx, lock_ether_rx;
	pthread_attr_t attr;
    priority_queue<lirc_packet> lirc_tx;
    priority_queue<lirc_packet> lirc_rx;
    priority_queue<lirc_packet> lirc_pending;
    priority_queue<ether_packet> ether_tx;
    priority_queue<ether_packet> ether_rx;
    lirc_packet ether_to_lirc(ether_packet& erp);
    ether_packet lirc_to_ether(lirc_packet& irp);
	int ether_crc(ether_packet& erp);
	int ir_dst(lirc_packet& irp);
	vector<unsigned char> addresses;
	vector<LightnetTap*> taps;
	vector<LightnetLIRC*> lircs;
	pthread_t p_threads[MAX_THREADS];// Threads
	int thread_count = 0;
};


class LightnetTap
{

  public:
    LightnetTap(Lightnet* ln) : lnet(ln) {};
    int init(unsigned char addr);
    void run();
	static void *helper(void *context) {((LightnetTap *)context)->run();};
  private:
    int tun_alloc(char *dev, int flags);
    int cread(int fd, char *buf, int n);
    int cwrite(int fd, char *buf, int n);
    int tap_fd;
	Lightnet* lnet;

};

class LightnetLIRC
{

  public:
    LightnetLIRC(Lightnet* ln) : lnet(ln) {};
    int init(string path);
    void run();
	static void *helper(void *context) {((LightnetLIRC *)context)->run();};
	const char unsigned pulse257[4] = {0x01, 0x01, 0x00, 0x00};
	const char unsigned pulse771[4] = {0x03, 0x03, 0x00, 0x00};
	const char unsigned space257[4] = {0x01, 0x01, 0x00, 0x00};
	const char unsigned flag[4] = {0xe8, 0x03, 0x00, 0x00};
  private:
    int IRfd;
	Lightnet* lnet;
	struct timeval IRtv; 
	fd_set IRfds;
   int getLength(char lbyte, char mbyte, char rbyte);
};
