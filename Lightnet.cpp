#include "Lightnet.h"

/**************************************************************************
 * ether_to_ir: converts ether packet to lirc packet                                         *
 **************************************************************************/
lirc_packet Lightnet::ether_to_lirc(ether_packet& erp) {
  lirc_packet irp;
  irp.type = DATA;
  //memcpy(irp.buff,erp.buff,4);//copy flags & proto
  //irp.buff[4]=erp.buff[9];//copy dst mac
  //irp.buff[5]=erp.buff[15];//copy src mac
  //memcpy(irp.buff+6,erp.buff+16,erp.length-16);//copy ethertype & data
  memcpy(irp.buff,erp.buff,erp.length);//copy flags & proto
  int id = rand();
  char bid[2];
  bid[0] = (unsigned char)id;
  bid[1] = (unsigned char)(id>>8);
  memcpy(irp.buff+erp.length,bid,2);//create rand id
  irp.length = erp.length + 2;
  if(crc)
    append_crc(irp);
  return irp;
}

/**************************************************************************
 * lirc_to_ether: converts lirc packet to ether packet                                         *
 **************************************************************************/
ether_packet Lightnet::lirc_to_ether(lirc_packet& irp) {
  if(crc)
    remove_crc(irp);
  ether_packet erp;
  //memcpy(erp.buff,irp.buff,4); //copy flags & proto
  //memcpy(erp.buff+4,ether_mac,5); //add dst mac
  //erp.buff[9] = irp.buff[4];
  //memcpy(erp.buff+10,ether_mac,5); //add src mac
  //erp.buff[15] = irp.buff[5];
  //memcpy(erp.buff+16,irp.buff+6,irp.length-2); //copy ethertype & data
  memcpy(erp.buff,irp.buff,irp.length);//copy flags & proto
  erp.length = irp.length - 2;
  return erp;
}

lirc_packet Lightnet::lirc_ack(lirc_packet& irp) {
  lirc_packet ir_ack;
  ir_ack.length = 4;
  ir_ack.buff[0] = irp.buff[5];//flip src & dst
  ir_ack.buff[1] = irp.buff[4];
  if(crc)
    memcpy(ir_ack.buff+2,irp.buff+irp.length-6,2); //copy id
  else
    memcpy(ir_ack.buff+2,irp.buff+irp.length-2,2); //copy id
  ir_ack.type=ACK;
  return ir_ack;
}


int Lightnet::lirc_dst(lirc_packet& irp)
{
  //for(int i = 0; i < addresses.size(); i++)
  //  if(addresses[i] == irp.buff[0])
  //	  return 1;
  return 1;
}

int Lightnet::check_crc(lirc_packet& irp) {
	if(crc == 0)
		return 1;
		
    unsigned long checkSum = 0, bufNum = 0;
	for(int i = 0; i < irp.length; i++){
		bufNum = 0;
		if(i+3<irp.length)
			bufNum = ((int)irp.buff[i])<<24+((int)irp.buff[i+1])<<16+((int)irp.buff[i+2])<<8+((int)irp.buff[i+3]);
		else if(i+2<irp.length)
			bufNum = ((int)irp.buff[i])<<24+((int)irp.buff[i+1])<<16+((int)irp.buff[i+2])<<8;
		else if(i+1<irp.length)
			bufNum = ((int)irp.buff[i])<<24+((int)irp.buff[i+1])<<16;
		else if(i<irp.length)
			bufNum = ((int)irp.buff[i])<<24;
		checkSum = checkSum^bufNum;
	}
	
	if(checkSum == 0)
		return 1;
	else
		return 0;
}

void Lightnet::append_crc(lirc_packet& irp){
	char checkSum[4] = {0,0,0,0};
	int end = irp.length + (irp.length % 4);
	for(int i = irp.length; i < end; i++)
		irp.buff[i] = 0x00;
	
	for(int i = 0; i < end; i++){
		checkSum[i%4] = (checkSum[i%4])^(irp.buff[i]);
		cerr << checkSum[i%4] << " ";
	}
	
	memcpy(irp.buff+irp.length,checkSum,4);
	cerr << endl << checkSum[0] << checkSum[1] << checkSum[2] << checkSum[3] << endl;
	irp.length += 4;
}

void Lightnet::remove_crc(lirc_packet& irp){
	irp.length -= 4;
}

int Lightnet::lirc_id(lirc_packet& p)
{
  int start = p.length-2;
  if(crc && p.type == DATA)
    start = p.length-6;
  return (int)(p.buff[start])<<8+(int)(p.buff[start+1]);
}

void Lightnet::remove_pending(lirc_packet& ir_ack)
{
  int id = lirc_id(ir_ack);
  pthread_mutex_lock(&lock_lirc_pending);
  for(int i = 0; i < lirc_pending.size(); i++)
    if(lirc_id(lirc_pending[i])==id)
	{
	  lirc_pending.erase(lirc_pending.begin()+i);
	  break;
	}
  pthread_mutex_unlock(&lock_lirc_pending);
}

void Lightnet::clear_pending()
{
  struct timeval current;
  gettimeofday(&current, NULL);
  pthread_mutex_lock(&lock_lirc_pending);
  for(int i = 0; i < lirc_pending.size(); i++)
  {
    //cerr << "pending time: " << (current.tv_sec-lirc_pending[i].sent.tv_sec) << endl;
    if((current.tv_sec-lirc_pending[i].sent.tv_sec) >= timeout)
      if(lirc_pending[i].transmissions>=transmissions)
	    lirc_pending.erase(lirc_pending.begin()+i--);
	  else
	  {
	    push_lirc_tx(lirc_pending[i]);
		lirc_pending.erase(lirc_pending.begin()+i--);
	  }
  }
  pthread_mutex_unlock(&lock_lirc_pending);
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
  srand(time(NULL));
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
  lirc_packet tmp = lirc_pending.front();
  lirc_pending.erase(lirc_pending.begin());
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
  lirc_pending.push_back(p);
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

int Lightnet::init(unsigned char addr, string path)
{
  LightnetTap* ltap = new LightnetTap(this);
  LightnetLIRC* llirc = new LightnetLIRC(this);
  this->multithread = 0;
  if(ltap->init(addr))
  {
    taps.push_back(ltap);
	addresses.push_back(addr);
	if(llirc->init(path))
    {
      lircs.push_back(llirc);
	  return 1;
    }
  }else
    return 0;
}


void Lightnet::run()
{
  cerr << "net start\n";
  int loop = 0;
  while(1)
  {
    if(debugMain>6)
		cerr << "LIRC Itter" << endl;
    if(multithread==0)
		lircs[0]->iteration();
  
    if(debugMain>6)
		cerr << "LIRC Rx" << endl;
    while(!empty_lirc_rx())
	{
	  lirc_packet ir_tmp = pop_lirc_rx();
	  cerr << "irpacket: " << ir_tmp.type << " " << ir_tmp.length << endl;
	  if(lirc_dst(ir_tmp))
	    if(ir_tmp.type == ACK && ir_tmp.length == 4)
		{
		  remove_pending(ir_tmp);
		  cerr << empty_lirc_pending() << endl;
		}
	    if(ir_tmp.type == DATA && ir_tmp.length > 18)
	    {
	      
	      if(check_crc(ir_tmp))
	      {
		    ether_packet ether_tmp = lirc_to_ether(ir_tmp);
	        push_ether_tx(ether_tmp);
	        lirc_packet ir_ack = lirc_ack(ir_tmp);
	        push_lirc_tx(ir_ack);
	      }
	    }
	}
	
	 if(debugMain>6)
		cerr << "LIRC Itter" << endl;
	if(multithread==0)
		lircs[0]->iteration();
	 if(debugMain>6)
		cerr << "Ether Itter" << endl;
	if(multithread==0)
		taps[0]->iteration();
	

	while(!empty_ether_rx())
	{
		if(debugMain>6)
			cerr << "Ether Rx" << endl;
		ether_packet ether_tmp = pop_ether_rx();
		cerr << "hit" << endl;
		lirc_packet lirc_tmp = ether_to_lirc(ether_tmp);
		cerr << "hit2" << endl;
		push_lirc_tx(lirc_tmp);
		cerr << "done" << endl;
	}
	

	if(!empty_lirc_pending())
	{
		if(debugMain>6)
			cerr << "Pending Cleanup" << endl;
		clear_pending();
	}
	loop++;
	cerr << "loop" << loop << "\n";
	

	
  }
  cerr << "end";

}
