#include "Lightnet.h"

/**************************************************************************
 * ether_to_ir: converts ether packet to lirc packet                                         *
 **************************************************************************/
void Lightnet::ether_to_lirc(Packet& p) {
  if(p.type != ETHERNET)
    cerr << "ETHER_TO_LIRC: Wrong Packet Type\n";
  p.type = LIRCDATA;
  int id = rand();
  p.buff[p.length] = (unsigned char)id;
  p.buff[p.length+1] = (unsigned char)(id>>8);
  p.buff[p.length+2] = (unsigned char)(id>>16);
  p.buff[p.length+3] = (unsigned char)(id>>24);
  p.length += 4;
  if(crc)
    append_crc(p);
}

/**************************************************************************
 * lirc_to_ether: converts lirc packet to ether packet                                         *
 **************************************************************************/
void Lightnet::lirc_to_ether(Packet& p) {
  if(p.type != LIRCDATA)
    cerr << "LIRC_TO_ETHER: Wrong Packet Type\n";
  p.type = ETHERNET;
  if(crc)
    remove_crc(p);
  p.length -= 4;
}

Packet Lightnet::lirc_ack(Packet& p) {
  if(p.type != LIRCDATA)
    cerr << "LIRC_ACK: Wrong Packet Type\n";
  Packet ir_ack;
  ir_ack.length = 10;
  memcpy(ir_ack.buff,p.buff+10,6);
  if(crc)
    memcpy(ir_ack.buff+4,p.buff+p.length-8,4); //copy id
  else
    memcpy(ir_ack.buff+4,p.buff+p.length-4,4); //copy id
  ir_ack.type=LIRCACK;
  return ir_ack;
}

//********************************************************************************************************************
/*int Lightnet::lirc_dst(Packet& p)
{
  
  for(int i = 0; i < addresses.size(); i++)
    if(addresses[i] == p.buff[0])
  	  return 1;
  return 1;
}*/

int Lightnet::check_crc(Packet& p) {
	if(crc == 0 || p.type != LIRCDATA)
		return 1;
		
    /*unsigned long checkSum = 0, bufNum = 0;
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
	else*/
		return 0;
}

void Lightnet::append_crc(Packet& p){
	if(p.type == LIRCDATA)
	{
		/*char checkSum[4] = {0,0,0,0};
		int end = irp.length + (irp.length % 4);
		for(int i = irp.length; i < end; i++)
			irp.buff[i] = 0x00;
		
		for(int i = 0; i < end; i++){
			checkSum[i%4] = (checkSum[i%4])^(irp.buff[i]);
			cerr << checkSum[i%4] << " ";
		}
		
		memcpy(irp.buff+irp.length,checkSum,4);
		cerr << endl << checkSum[0] << checkSum[1] << checkSum[2] << checkSum[3] << endl;*/
		p.length += 4;
	}
}
//********************************************************************************************************************
void Lightnet::remove_crc(Packet& p){
	if(p.type == LIRCDATA)
		p.length -= 4;
}

int Lightnet::lirc_id(Packet& p)
{
  if(p.type == ETHERNET)
	cerr << "LIRC_ID: Wrong Packet Type\n";
  int start = p.length-4;
  if(crc && p.type == LIRCDATA)
    start = p.length-10;
  return (int)(p.buff[start])<<24+(int)(p.buff[start+1])<<16+(int)(p.buff[start+2])<<8+(int)(p.buff[start+3]);
}

void Lightnet::remove_pending(Packet& ir_ack)
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
  //unsigned char broadcast = 0xFF;
  //addresses.push_back(broadcast);
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

Packet Lightnet::pop_lirc_tx()
{
  pthread_mutex_lock(&lock_lirc_tx);
  Packet tmp = lirc_tx.top();
  lirc_tx.pop();
  pthread_mutex_unlock(&lock_lirc_tx);
  return tmp;
}

Packet Lightnet::pop_lirc_rx()
{
  pthread_mutex_lock(&lock_lirc_rx);
  Packet tmp = lirc_rx.top();
  lirc_rx.pop();
  pthread_mutex_unlock(&lock_lirc_rx);
  return tmp;
}

Packet Lightnet::pop_lirc_pending()
{
  pthread_mutex_lock(&lock_lirc_pending);
  Packet tmp = lirc_pending.front();
  lirc_pending.erase(lirc_pending.begin());
  pthread_mutex_unlock(&lock_lirc_pending);
  return tmp;
}

Packet Lightnet::pop_ether_tx()
{
  pthread_mutex_lock(&lock_ether_tx);
  Packet tmp = ether_tx.top();
  ether_tx.pop();
  pthread_mutex_unlock(&lock_ether_tx);
  return tmp;
}

Packet Lightnet::pop_ether_rx()
{
  pthread_mutex_lock(&lock_ether_rx);
  Packet tmp = ether_rx.top();
  ether_rx.pop();
  pthread_mutex_unlock(&lock_ether_rx);
  return tmp;
}

void Lightnet::push_lirc_tx(Packet p)
{
  pthread_mutex_lock(&lock_lirc_tx);
  lirc_tx.push(p);
  pthread_mutex_unlock(&lock_lirc_tx);
}

void Lightnet::push_lirc_rx(Packet p)
{
  pthread_mutex_lock(&lock_lirc_rx);
  lirc_rx.push(p);
  pthread_mutex_unlock(&lock_lirc_rx);
}

void Lightnet::push_lirc_pending(Packet p)
{
  pthread_mutex_lock(&lock_lirc_pending);
  lirc_pending.push_back(p);
  pthread_mutex_unlock(&lock_lirc_pending);
}

void Lightnet::push_ether_tx(Packet p)
{
  pthread_mutex_lock(&lock_ether_tx);
  ether_tx.push(p);
  pthread_mutex_unlock(&lock_ether_tx);
}

void Lightnet::push_ether_rx(Packet p)
{
  pthread_mutex_lock(&lock_ether_rx);
  ether_rx.push(p);
  pthread_mutex_unlock(&lock_ether_rx);
}


int Lightnet::init_tap(unsigned char addr)
{
  LightnetTAP* ltap = new LightnetTAP(this);
  if(ltap->init(addr))
  {
    taps.push_back(ltap);
	//addresses.push_back(addr);
	pthread_create(&p_threads[thread_count++], &attr, &LightnetTAP::helper, ltap);
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
  LightnetTAP* ltap = new LightnetTAP(this);
  LightnetLIRC* llirc = new LightnetLIRC(this);
  this->multithread = 0;
  if(ltap->init(addr))
  {
    taps.push_back(ltap);
	//addresses.push_back(addr);
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
    if(debug_main>6)
		cerr << "LIRC Itter" << endl;
    if(multithread==0)
		lircs[0]->iteration();
  
    if(debug_main>6)
		cerr << "LIRC Rx" << endl;
    while(!empty_lirc_rx())
	{
	  Packet tmp = pop_lirc_rx();
	  cerr << "irpacket: " << tmp.type << " " << tmp.length << endl;
	  //if(lirc_dst(ir_tmp))
	    if(tmp.type == LIRCACK && tmp.length == 10)
		{
		  remove_pending(tmp);
		  cerr << empty_lirc_pending() << endl;
		}
	    if(tmp.type == LIRCDATA && tmp.length > 22)
	    {
	      
	      if(check_crc(tmp))
	      {
	        Packet ir_ack = lirc_ack(tmp);
	        push_lirc_tx(ir_ack);
			lirc_to_ether(tmp);
			push_ether_tx(tmp);
	      }
	    }
	}
	
	 if(debug_main>6)
		cerr << "LIRC Itter" << endl;
	if(multithread==0)
		lircs[0]->iteration();
	 if(debug_main>6)
		cerr << "Ether Itter" << endl;
	if(multithread==0)
		taps[0]->iteration();
	

	while(!empty_ether_rx())
	{
		if(debug_main>6)
			cerr << "Ether Rx" << endl;
		Packet tmp = pop_ether_rx();
		cerr << "hit" << endl;
		ether_to_lirc(tmp);
		cerr << "hit2" << endl;
		push_lirc_tx(tmp);
		cerr << "done" << endl;
	}
	

	if(!empty_lirc_pending())
	{
		if(debug_main>6)
			cerr << "Pending Cleanup" << endl;
		clear_pending();
	}
	loop++;
	cerr << "loop" << loop << "\n";
	

	
  }
  cerr << "end";

}
