#include "Lightnet.h"

/**************************************************************************
 * ether_to_ir: converts ether packet to lirc packet                                         *
 **************************************************************************/
lirc_packet Lightnet::ether_to_ir(ether_packet& erp) {
  lirc_packet irp;
  irp.buff[0]=erp.buff[9];//copy dst mac
  irp.buff[1]=erp.buff[15];//copy src mac
  strncpy(irp.buff+2,erp.buff+16,erp.length-16);//copy ethertype & data & crc
  irp.length = erp.length - 14;
}

/**************************************************************************
 * lirc_to_ether: converts lirc packet to ether packet                                         *
 **************************************************************************/
ether_packet Lightnet::lirc_to_ether(lirc_packet& irp) {
  ether_packet erp;
  strncpy(erp.buff,reinterpret_cast<const char*>(ether_flag),4);//add start flag
  strncpy(erp.buff+4,reinterpret_cast<const char*>(ether_mac),5); //add dst mac
  erp.buff[9] = irp.buff[0];
  strncpy(erp.buff+10,reinterpret_cast<const char*>(ether_mac),5); //add src mac
  erp.buff[15] = irp.buff[1];
  strncpy(erp.buff+16,irp.buff+2,irp.length-2); //copy ethertype & data & crc
  erp.length = irp.length + 14;
}

int Lightnet::ir_dst(lirc_packet& irp)
{
  for(int i = 0; i < addresses.size(); i++)
    if(addresses[i] == irp.buff[0])
	  return 1;
  return 0;
}

/**************************************************************************
 * ether_crc: crc ether packet                                         *
 **************************************************************************/
int Lightnet::ether_crc(ether_packet& erp) {
  return 1;
}

Lightnet::Lightnet()
{
  pthread_mutex_init(&lock_lirc_tx, NULL);
  pthread_mutex_init(&lock_lirc_rx, NULL); 
  pthread_mutex_init(&lock_lirc_pending, NULL); 
  pthread_mutex_init(&lock_ether_tx, NULL); 
  pthread_mutex_init(&lock_ether_rx, NULL); 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  unsigned char broadcast = 0xFF;
  addresses.push_back(broadcast);
}

Lightnet::~Lightnet()
{
  pthread_mutex_destroy(&lock_lirc_tx);
  pthread_mutex_destroy(&lock_lirc_rx); 
  pthread_mutex_destroy(&lock_lirc_pending); 
  pthread_mutex_destroy(&lock_ether_tx); 
  pthread_mutex_destroy(&lock_ether_rx);
  pthread_attr_destroy(&attr);  
  for(int i = 0; i < taps.size(); i++)
    delete taps[i];
  for(int i = 0; i < lircs.size(); i++)
    delete lircs[i];
}

int Lightnet::empty_lirc_tx()
{
  pthread_mutex_lock(&lock_lirc_tx);
  int tmp = lirc_tx.empty();
  pthread_mutex_unlock(&lock_lirc_tx);
  return tmp;
}

int Lightnet::empty_lirc_rx()
{
  pthread_mutex_lock(&lock_lirc_rx);
  int tmp = lirc_rx.empty();
  pthread_mutex_unlock(&lock_lirc_rx);
  return tmp;
}

int Lightnet::empty_lirc_pending()
{
  pthread_mutex_lock(&lock_lirc_pending);
  int tmp = lirc_pending.empty();
  pthread_mutex_unlock(&lock_lirc_pending);
  return tmp;
}

int Lightnet::empty_ether_tx()
{
  pthread_mutex_lock(&lock_ether_tx);
  int tmp = ether_tx.empty();
  pthread_mutex_unlock(&lock_ether_tx);
  return tmp;
}

int Lightnet::empty_ether_rx()
{
  pthread_mutex_lock(&lock_ether_rx);
  int tmp = ether_rx.empty();
  pthread_mutex_unlock(&lock_ether_rx);
  return tmp;
}

lirc_packet Lightnet::pop_lirc_tx()
{
  pthread_mutex_lock(&lock_lirc_tx);
  lirc_packet tmp = lirc_tx.top();
  lirc_tx.pop();
  pthread_mutex_unlock(&lock_lirc_tx);
  return tmp;
}

lirc_packet Lightnet::pop_lirc_rx()
{
  pthread_mutex_lock(&lock_lirc_rx);
  lirc_packet tmp = lirc_rx.top();
  lirc_rx.pop();
  pthread_mutex_unlock(&lock_lirc_rx);
  return tmp;
}

lirc_packet Lightnet::pop_lirc_pending()
{
  pthread_mutex_lock(&lock_lirc_pending);
  lirc_packet tmp = lirc_pending.top();
  lirc_pending.pop();
  pthread_mutex_unlock(&lock_lirc_pending);
  return tmp;
}

ether_packet Lightnet::pop_ether_tx()
{
  pthread_mutex_lock(&lock_ether_tx);
  ether_packet tmp = ether_tx.top();
  ether_tx.pop();
  pthread_mutex_unlock(&lock_ether_tx);
  return tmp;
}

ether_packet Lightnet::pop_ether_rx()
{
  pthread_mutex_lock(&lock_ether_rx);
  ether_packet tmp = ether_rx.top();
  ether_rx.pop();
  pthread_mutex_unlock(&lock_ether_rx);
  return tmp;
}

void Lightnet::push_lirc_tx(lirc_packet p)
{
  pthread_mutex_lock(&lock_lirc_tx);
  lirc_tx.push(p);
  pthread_mutex_unlock(&lock_lirc_tx);
}

void Lightnet::push_lirc_rx(lirc_packet p)
{
  pthread_mutex_lock(&lock_lirc_rx);
  lirc_rx.push(p);
  pthread_mutex_unlock(&lock_lirc_rx);
}

void Lightnet::push_lirc_pending(lirc_packet p)
{
  pthread_mutex_lock(&lock_lirc_pending);
  lirc_pending.push(p);
  pthread_mutex_unlock(&lock_lirc_pending);
}

void Lightnet::push_ether_tx(ether_packet p)
{
  pthread_mutex_lock(&lock_ether_tx);
  ether_tx.push(p);
  pthread_mutex_unlock(&lock_ether_tx);
}

void Lightnet::push_ether_rx(ether_packet p)
{
  pthread_mutex_lock(&lock_ether_rx);
  ether_rx.push(p);
  pthread_mutex_unlock(&lock_ether_rx);
}


int Lightnet::init_tap(unsigned char addr)
{
  LightnetTap* ltap = new LightnetTap(this);
  if(ltap->init(addr))
  {
    taps.push_back(ltap);
	addresses.push_back(addr);
	pthread_create(&p_threads[thread_count++], &attr, &LightnetTap::helper, ltap);
	return 1;
  }else
    return 0;
}

int Lightnet::init_lirc(string path)
{
  LightnetLIRC* llirc = new LightnetLIRC(this);
  if(llirc->init(path))
  {
    lircs.push_back(llirc);
	pthread_create(&p_threads[thread_count++], &attr, &LightnetLIRC::helper, llirc);
	return 1;
  }else
    return 0;
}

void Lightnet::run()
{
  cerr << "net start\n";
  int loop = 0;
  while(1)
  {
    while(!empty_lirc_rx())
	{
	  lirc_packet ir_tmp = pop_lirc_rx();
	  if(ir_dst(ir_tmp))
	    if(ir_tmp.type == DATA)
	    {
	      ether_packet ether_tmp = lirc_to_ether(ir_tmp);
	      if(ether_crc(ether_tmp))
	      {
	        push_ether_tx(ether_tmp);
	        lirc_packet ir_ack;
		    ir_ack.length = 2;
		    ir_ack.buff[0] = ir_tmp.buff[1];
		    ir_ack.buff[1] = ir_tmp.buff[0];
		    ir_ack.type=ACK;
	        push_lirc_tx(ir_ack);
	      }
	    }
	}
	
	//cerr << "loop\n";
	while(!empty_ether_rx())
	{
	  ether_packet ether_tmp = pop_ether_rx();
	  //cerr << "packet data = ";
	  cerr << "~" << ether_tmp.length << "\n";
	  push_lirc_tx(ether_to_ir(ether_tmp));
	}
	
	while(loop > 9 && !empty_lirc_pending())
	{
	  lirc_packet ir_tmp = pop_lirc_pending();
	  struct timeval current;
	  gettimeofday(&current, NULL);
	  if(((current.tv_sec-ir_tmp.sent.tv_sec)+0.000001*(current.tv_usec-ir_tmp.sent.tv_usec)) > timeout)
	    push_lirc_tx(ir_tmp);
	  else
	    push_lirc_pending(ir_tmp);
	}
	
	usleep(100000);
	if(loop > 9)
	  loop = 0;
  }
  cerr << "end";

}


int main(int argc, char *argv[])
{
  Lightnet l;
  l.init_tap(0x01);
  cerr << "hit\n";
  l.init_lirc("/dev/lirc0");
  cerr << "hit\n";
  l.run();
}