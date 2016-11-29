#include "Lightnet.h"

void printBuffer(char buffer[],int size)
{
	for (int i=0; i<size; i++) {
		/*int charInt = buffer[i];
		int j;
                for (int j=0; j<8; j++) {
                    if (charInt >= (1<<(7-j))) {
                           // write a pulse 1
                            cerr << "1";
                            charInt = charInt - (1<<(7-j));
                    } else {
                            // write a pulse 0
                            cerr << "0";
                    }
                }*/
		printf("%02x|",buffer[i] & 0xff);
	}
    cerr << endl;
}

Packet mkpacket(unsigned char dst,unsigned char src, Lightnet& l)
{
	Packet erp;
	erp.type = ETHERNET;
	char packet[] = "This is the packet that will be sent in chunks of 63 bytes across the IR network to the target device. Thanks to the changes that I just made, I am now able to send much larger packets.";
	int size = sizeof(packet) - 1;
	//printf("%s %i\n",packet, size);
	erp.buff[0] = 0x00;//fake flags & proto
	erp.buff[1] = 0x00;
	erp.buff[2] = 0x00;
	erp.buff[3] = 0x00;
	memcpy(erp.buff+4,l.ether_mac,5); //add dst mac
	erp.buff[9] = dst;
	memcpy(erp.buff+10,l.ether_mac,5); //add src mac
	erp.buff[15] = src;
	erp.buff[16] = 0x00;//fake ethertype
	erp.buff[17] = 0x00;
	memcpy(erp.buff+18,packet,size); //copy data
	erp.length = size+18;
	return erp;
}

void rdpacket(Packet& erp)
{
	cout << (int)erp.buff[9] << "-" << (int)erp.buff[15] << "=" << erp.length << endl;
	char tmp[BUFSIZE];
	memcpy(tmp,erp.buff+18,erp.length-18); //copy data
	tmp[erp.length] = '\0';
	printf("%s\n",tmp);

}

int main(int argc, char *argv[])
{
  Lightnet l;
//  l.init_tap(0x01);
//  l.taps[0]->run();
  //cerr << "hit\n";
  
  l.init(0x01,"/dev/lirc0");
  
  cerr << "start\n";
  int loop = 0;
  while(1)
  {
    cerr << "LIRC Itter" << endl;
    l.lircs[0]->iteration();
	cerr << "LIRC Rx" << endl;
  
    while(!l.empty_lirc_rx())
	{
	  Packet tmp = l.pop_lirc_rx();
	  cerr << "irpacket: " << tmp.type << " " << tmp.length << endl;
	  //printBuffer(tmp.buff,tmp.length-8);
	  //printBuffer(tmp.buff+tmp.length-8,4);
	  //printBuffer(tmp.buff+tmp.length-4,4);
	 
	    if(tmp.type == LIRCACK && tmp.length == 10)
		{
		  printBuffer(tmp.buff,tmp.length);
		  cerr << "remove_pending" << endl;
		  l.remove_pending(tmp);
		  cerr << l.empty_lirc_pending() << endl;
		}
	    if(tmp.type == LIRCDATA && tmp.length > 22)
	    {
	      
	      if(l.check_crc(tmp))
	      {
			if(l.check_unicast(tmp))
			{
	          Packet tmp_ack = l.lirc_ack(tmp);
			  cerr << "ack\n";
			  printBuffer(tmp.buff,tmp.length);
			  printBuffer(tmp_ack.buff,tmp_ack.length);
			  cerr << "ackend\n";
			  l.push_lirc_tx(tmp_ack);
			}
			l.lirc_to_ether(tmp);
			cout << "rd\n";
			rdpacket(tmp);

	      }
	    }
	}
	
	cerr << "LIRC Itter" << endl;
	l.lircs[0]->iteration();
	cerr << "Ether Itter" << endl;
	//l.taps[0]->iteration();
	
	while(!l.empty_ether_rx())
	{
		cerr << "Ether Rx" << endl;
		Packet tmp = l.pop_ether_rx();
		cerr << "hit" << endl;
		l.ether_to_lirc(tmp);
		cerr << "hit2" << endl;
	    printBuffer(tmp.buff,tmp.length-8);
	    //printBuffer(tmp.buff+tmp.length-8,4);
	    //printBuffer(tmp.buff+tmp.length-4,4);
		l.push_lirc_tx(tmp);
		cerr << "done" << endl;
	}
	
	if(!l.empty_lirc_pending())
	{
		cerr << "Pending Cleanup" << endl;
		l.clear_pending();
	}
	
	if(loop % 20 == 0)//create packet to send and decode
	{
	 Packet tmp = mkpacket(0xff,0x1,l);
	  l.push_ether_rx(tmp);
	}
	loop++;
	cerr << "loop" << loop << "\n";
	//l.taps[0]->iteration();
	
  }
  cerr << "end";

}
