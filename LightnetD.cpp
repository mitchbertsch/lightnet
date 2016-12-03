#include "Lightnet.h"
using boost::property_tree::ptree;
using boost::property_tree::read_json;


int main(int argc, char *argv[])
{
	string config = "../lightnet.json";
	fstream file(config);
	if(!file.is_open())
	{
		cerr << "Config File Missing: " << config << endl;
		exit(1);
	}
	ptree pt;
	read_json(file,pt);
	
	//create Lightnet object
	Lightnet l;
	unsigned char tap = 0x01;
	string lirc = "/dev/lirc0";
	
	//parse main thread debug
	if(pt.get_child_optional("debug"))
		l.debug = pt.get<int>("debug");
		
	//open syslog
	openlog("lightnetd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	
	//parse mtu size
	if(pt.get_child_optional("mtu"))
		l.mtu = pt.get<int>("mtu");
	syslog(LOG_INFO, "MTU set to: %d", l.mtu);
		
	//parse crc
	if(pt.get_child_optional("crc"))
		l.crc = pt.get<int>("crc");
	syslog(LOG_INFO, "CRC set to: %d", l.crc);
	
	//parse nodes
	if(pt.get_child_optional("nodes"))
		l.nodes = pt.get<int>("nodes");
	syslog(LOG_INFO, "Nodes set to: %d", l.nodes);
	
	//parse multithread
	if(pt.get_child_optional("multithread"))
		l.multithread = pt.get<int>("multithread");
	syslog(LOG_INFO, "Multithread set to: %d", l.mtu);
		
	//parse timeout
	if(pt.get_child_optional("timeout"))
		l.timeout = pt.get<int>("timeout");
	syslog(LOG_INFO, "Timeout set to: %d", l.timeout);
		
	//parse transmissions
	if(pt.get_child_optional("transmissions"))
		l.transmissions = pt.get<int>("transmissions");
	syslog(LOG_INFO, "Transmissions set to: %d", l.transmissions);
			
	//parse ipv4
	if(pt.get_child_optional("ipv4"))
	{
		stringstream ss(pt.get<string>("ipv4"));
		int i = 0;
		string token;
		while(getline(ss,token,'.') && i < 3)
		{
			l.ipv4[i++]=(unsigned char)(stoi(token));
		}
	}
	syslog(LOG_INFO, "IPv4 network set to: %d.%d.%d.0", (int)l.ipv4[0],(int)l.ipv4[1],(int)l.ipv4[2]);
	
	//parse mac
	if(pt.get_child_optional("ethernet_mac"))
	{
		stringstream ss(pt.get<string>("ethernet_mac"));
		int i = 0;
		string token;
		while(getline(ss,token,':') && i < 5)
		{
			l.ether_mac[i++]=(unsigned char)(strtoul(token.c_str(),NULL,16));
		}
	}
	syslog(LOG_INFO, "Ethernet MAC set to: %x:%x:%x:%x:%x:00", (int)l.ether_mac[0],(int)l.ether_mac[1],(int)l.ether_mac[2],(int)l.ether_mac[3],(int)l.ether_mac[4]);

	//parse tap id
	if(pt.get_child_optional("tap.id"))
		tap = (unsigned char)(pt.get<int>("tap.id"));
	else
	{
		cerr << "No Tap ID Provided\n";
		exit(2);
	}
	syslog(LOG_INFO, "TAP ID set to: %d", (int)tap);
	
	//parse lirc path
	if(pt.get_child_optional("lirc.path"))
		lirc = pt.get<string>("lirc.path");
	else
	{
		cerr << "No LIRC Path Provided\n";
		exit(3);
	}
	syslog(LOG_INFO, "LIRC Path set to: %s", lirc.c_str());
		
	if(l.multithread == 1)
	{
		l.init_tap(tap);
		l.init_lirc(lirc);
	}
	else
		l.init(tap,lirc);
		
			
	//parse lirc space
	if(pt.get_child_optional("lirc.space"))
	{
		int space = pt.get<int>("lirc.space");
		l.lircs[0]->space[0] = space & 0xff;
		l.lircs[0]->space[1] = (space>>8) & 0xff;
		l.lircs[0]->space[2] = (space>>16) & 0xff;
		l.lircs[0]->space[3] = (space>>24) & 0xff;
	}
	syslog(LOG_INFO, "LIRC Space set to: %x %x %x %x", (int)(l.lircs[0]->space[0]),(int)(l.lircs[0]->space[1]),(int)(l.lircs[0]->space[2]),(int)(l.lircs[0]->space[3]));
	
	//parse lirc pulse_zero
	if(pt.get_child_optional("lirc.pulse_zero"))
	{
		int pulse_zero = pt.get<int>("lirc.pulse_zero");
		l.lircs[0]->pulse_zero[0] = pulse_zero & 0xff;
		l.lircs[0]->pulse_zero[1] = (pulse_zero>>8) & 0xff;
		l.lircs[0]->pulse_zero[2] = (pulse_zero>>16) & 0xff;
		l.lircs[0]->pulse_zero[3] = (pulse_zero>>24) & 0xff;
	}
	syslog(LOG_INFO, "LIRC Pulse Zero set to: %x %x %x %x", (int)(l.lircs[0]->pulse_zero[0]),(int)(l.lircs[0]->pulse_zero[1]),(int)(l.lircs[0]->pulse_zero[2]),(int)(l.lircs[0]->pulse_zero[3]));
		
	//parse lirc pulse_one
	if(pt.get_child_optional("lirc.pulse_one"))
	{
		int pulse_one = pt.get<int>("lirc.pulse_one");
		l.lircs[0]->pulse_one[0] = pulse_one & 0xff;
		l.lircs[0]->pulse_one[1] = (pulse_one>>8) & 0xff;
		l.lircs[0]->pulse_one[2] = (pulse_one>>16) & 0xff;
		l.lircs[0]->pulse_one[3] = (pulse_one>>24) & 0xff;
	}
	syslog(LOG_INFO, "LIRC Pulse One set to: %x %x %x %x", (int)(l.lircs[0]->pulse_one[0]),(int)(l.lircs[0]->pulse_one[1]),(int)(l.lircs[0]->pulse_one[2]),(int)(l.lircs[0]->pulse_one[3]));
		
	//parse lirc pulse_data_flag
	if(pt.get_child_optional("lirc.pulse_data_flag"))
	{
		int pulse_data_flag = pt.get<int>("lirc.pulse_data_flag");
		l.lircs[0]->pulse_data_flag[0] = pulse_data_flag & 0xff;
		l.lircs[0]->pulse_data_flag[1] = (pulse_data_flag>>8) & 0xff;
		l.lircs[0]->pulse_data_flag[2] = (pulse_data_flag>>16) & 0xff;
		l.lircs[0]->pulse_data_flag[3] = (pulse_data_flag>>24) & 0xff;
	}
	syslog(LOG_INFO, "LIRC Pulse Data Flag set to: %x %x %x %x", (int)(l.lircs[0]->pulse_data_flag[0]),(int)(l.lircs[0]->pulse_data_flag[1]),(int)(l.lircs[0]->pulse_data_flag[2]),(int)(l.lircs[0]->pulse_data_flag[3]));
		
	//parse lirc pulse_sub_flag
	if(pt.get_child_optional("lirc.pulse_sub_flag"))
	{
		int pulse_sub_flag = pt.get<int>("lirc.pulse_sub_flag");
		l.lircs[0]->pulse_sub_flag[0] = pulse_sub_flag & 0xff;
		l.lircs[0]->pulse_sub_flag[1] = (pulse_sub_flag>>8) & 0xff;
		l.lircs[0]->pulse_sub_flag[2] = (pulse_sub_flag>>16) & 0xff;
		l.lircs[0]->pulse_sub_flag[3] = (pulse_sub_flag>>24) & 0xff;
	}
	syslog(LOG_INFO, "LIRC Pulse Sub Flag set to: %x %x %x %x", (int)(l.lircs[0]->pulse_sub_flag[0]),(int)(l.lircs[0]->pulse_sub_flag[1]),(int)(l.lircs[0]->pulse_sub_flag[2]),(int)(l.lircs[0]->pulse_sub_flag[3]));
		
	//parse lirc pulse_ack_flag
	if(pt.get_child_optional("lirc.pulse_ack_flag"))
	{
		int pulse_ack_flag = pt.get<int>("lirc.pulse_ack_flag");
		l.lircs[0]->pulse_ack_flag[0] = pulse_ack_flag & 0xff;
		l.lircs[0]->pulse_ack_flag[1] = (pulse_ack_flag>>8) & 0xff;
		l.lircs[0]->pulse_ack_flag[2] = (pulse_ack_flag>>16) & 0xff;
		l.lircs[0]->pulse_ack_flag[3] = (pulse_ack_flag>>24) & 0xff;
	}
	syslog(LOG_INFO, "LIRC Pulse Ack Flag set to: %x %x %x %x", (int)(l.lircs[0]->pulse_ack_flag[0]),(int)(l.lircs[0]->pulse_ack_flag[1]),(int)(l.lircs[0]->pulse_ack_flag[2]),(int)(l.lircs[0]->pulse_ack_flag[3]));

	//parse lirc pulse_one_min
	if(pt.get_child_optional("lirc.pulse_one_min"))
		l.lircs[0]->pulse_one_min = pt.get<int>("lirc.pulse_one_min");
	syslog(LOG_INFO, "LIRC Pulse One Min set to: %d", l.lircs[0]->pulse_one_min);
			
	//parse lirc pulse_one_max
	if(pt.get_child_optional("lirc.pulse_one_max"))
		l.lircs[0]->pulse_one_max = pt.get<int>("lirc.pulse_one_max");
	syslog(LOG_INFO, "LIRC Pulse One Max set to: %d", l.lircs[0]->pulse_one_max);
			
	//parse lirc pulse_sub_flag_min
	if(pt.get_child_optional("lirc.pulse_sub_flag_min"))
		l.lircs[0]->pulse_sub_flag_min = pt.get<int>("lirc.pulse_sub_flag_min");
	syslog(LOG_INFO, "LIRC Pulse Sub Flag Min set to: %d", l.lircs[0]->pulse_sub_flag_min);
			
	//parse lirc pulse_sub_flag_max
	if(pt.get_child_optional("lirc.pulse_sub_flag_max"))
		l.lircs[0]->pulse_sub_flag_max = pt.get<int>("lirc.pulse_sub_flag_max");
	syslog(LOG_INFO, "LIRC Pulse Sub Flag Max set to: %d", l.lircs[0]->pulse_sub_flag_max);
	
	//parse lirc pulse_data_flag_min
	if(pt.get_child_optional("lirc.pulse_data_flag_min"))
		l.lircs[0]->pulse_data_flag_min = pt.get<int>("lirc.pulse_data_flag_min");
	syslog(LOG_INFO, "LIRC Pulse Data Flag Min set to: %d", l.lircs[0]->pulse_data_flag_min);
			
	//parse lirc pulse_data_flag_max
	if(pt.get_child_optional("lirc.pulse_data_flag_max"))
		l.lircs[0]->pulse_data_flag_max = pt.get<int>("lirc.pulse_data_flag_max");
	syslog(LOG_INFO, "LIRC Pulse Data Flag Min set to: %d", l.lircs[0]->pulse_data_flag_max);
			
	//parse lirc pulse_ack_flag_min
	if(pt.get_child_optional("lirc.pulse_ack_flag_min"))
		l.lircs[0]->pulse_ack_flag_min = pt.get<int>("lirc.pulse_ack_flag_min");
	syslog(LOG_INFO, "LIRC Pulse Ack Flag Min set to: %d", l.lircs[0]->pulse_ack_flag_min);
			
	//parse lirc pulse_ack_flag_max
	if(pt.get_child_optional("lirc.pulse_ack_flag_max"))
		l.lircs[0]->pulse_ack_flag_max = pt.get<int>("lirc.pulse_ack_flag_max");
	syslog(LOG_INFO, "LIRC Pulse Ack Flag Max set to: %d", l.lircs[0]->pulse_ack_flag_max);
	
	//start main thread
	l.run();
}
