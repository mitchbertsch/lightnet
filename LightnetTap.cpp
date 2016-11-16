#include "Lightnet.h"

/**************************************************************************
 * TAP Code derived from Davide Brini's simpletun.c TUN/TAP program       * *************************************************************************/ 

/**************************************************************************
 * tun_alloc: allocates or reconnects to a tun/tap device. The caller     *
 *            needs to reserve enough space in *dev.                      *
 **************************************************************************/
int LightnetTap::tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK)) < 0 ) {
    perror("Opening /dev/net/tun");
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags;

  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  strcpy(dev, ifr.ifr_name);

  return fd;
}

/**************************************************************************
 * cread: read routine that checks for errors and exits if an error is    *
 *        returned.                                                       *
 **************************************************************************/
int LightnetTap::cread(int fd, char *buf, int n){
  
  int nread;

  if((nread=read(fd, buf, n))<0){
    perror("Reading data");
	close(fd);
    exit(1);
  }
  return nread;
}

/**************************************************************************
 * cwrite: write routine that checks for errors and exits if an error is  *
 *         returned.                                                      *
 **************************************************************************/
int LightnetTap::cwrite(int fd, char *buf, int n){
  
  int nwrite;

  if((nwrite=write(fd, buf, n))<0){
    perror("Writing data");
	close(fd);
    exit(1);
  }
  return nwrite;
}


int LightnetTap::init(unsigned char addr) {
  /* Connect to the device */
  stringstream ss1;
  ss1 << (int)addr;
  string tap_name = "tap"+ss1.str();
  tap_fd = tun_alloc(const_cast<char*>(tap_name.c_str()), IFF_TAP);  /* tun interface */
  string command = "./tap.sh ";
  command += tap_name + " ";
  stringstream ss2, ss3, ss4, ss5;
  ss2 << lnet->mtu;
  command += ss2.str() + " ";
  ss3 << hex << setfill('0') << setw(2);
  ss3 << (int)(lnet->ether_mac[0]) << ":";
  ss3 << (int)(lnet->ether_mac[1]) << ":";
  ss3 << (int)(lnet->ether_mac[2]) << ":";
  ss3 << (int)(lnet->ether_mac[3]) << ":";
  ss3 << (int)(lnet->ether_mac[4]) << ":";
  ss4 << hex << setfill('0') << setw(2) << (int)addr;
  command += ss3.str() + ss4.str() + " ";
  ss5 << (int)(lnet->ipv4[0]) << "."; 
  ss5 << (int)(lnet->ipv4[1]) << ".";
  ss5 << (int)(lnet->ipv4[2]) << ".";;
  command += ss5.str()+ss1.str() + " ";
  cerr << command << endl;
  system(command.c_str());
  usleep(10000);
  string command2;
  for(int i = 1; i <= lnet->nodes; i++)
    if(i != (int)addr)
    {
      stringstream ss6, ss7;
	  ss6 << (int)i;
	  ss7 << hex << setfill('0') << setw(2) << (int)i;
	  command2 = "./arp.sh " + tap_name + " " + ss5.str() + ss6.str() + " " + ss3.str() + ss7.str();
	  cerr << command2 << endl;
	  system(command2.c_str());
      usleep(10000);
    }
  if(tap_fd < 0){
    perror("Allocating interface");
	return tap_fd;
  }
  return 1;
}

void LightnetTap::iteration() {
  /* Now read data coming from the kernel */
    nread = read(tap_fd,buffer,sizeof(buffer));
	//cout << "tap loop\n";
    if(nread > 0) {
	  ether_packet tmp;
      tmp.length=nread;
      memcpy(tmp.buff,buffer,nread);
	  lnet->push_ether_rx(tmp);
	  cout << "`" << tmp.length << "\n";
    }
	

    while(!lnet->empty_ether_tx())
	{
	  ether_packet tmp = lnet->pop_ether_tx();
	  memcpy(buffer,tmp.buff,tmp.length);
	  nwrite = write(tap_fd, buffer, tmp.length);
	  //if(nwrite != tmp.length)
        
	  
	}
}


void LightnetTap::run() {
  cout << "tap start\n";
  /* Now read data coming from the kernel */
  while(1) {
    iteration();
    pthread_yield();
  }
}