#include "Lightnet.h"
int main(int argc, char *argv[])
{
  Lightnet l;
//  l.init_tap(0x01);
//  l.taps[0]->run();
  //cerr << "hit\n";
  char tpacket[] = "This is the packet that will be sent in chunks of 63 bytes across the IR network to the target device. Thanks to the changes that I just made, I am now able to send much larger packets.";
  int packetSize = sizeof(tpacket) - 1;
  l.init_lirc("/dev/lirc0");
  cerr << "hit\n";
  
  cerr << "start\n";
  int loop = 0;
  while(1)
  {
    while(!l.empty_lirc_rx())
	{
	  lirc_packet ir_tmp = l.pop_lirc_rx();
		  cerr << "irpacket: " << ir_tmp.type << endl;
	    if(ir_tmp.type == DATA)
	    {
		  ir_tmp.buff[ir_tmp.length] = '\0';
	      printf("%s\n",ir_tmp.buff);
	    
	    }
	}
	
	usleep(1000000);//force thread to run at 10Hz
	if(loop >= 10)
	{
	  lirc_packet ir_tmp;
	  memcpy(ir_tmp.buff,tpacket,packetSize);
	  ir_tmp.type = DATA;
	  ir_tmp.length = packetSize;
	  l.push_lirc_tx(ir_tmp);
	  loop = 0;
	}
	loop++;
	cerr << "loop" << loop << "\n";
  }
  cerr << "end";

}
