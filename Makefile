CXXFlags = -std=c++11 -lpthread
CXX = g++

all: lightnetd tester profiler

Lightnet.o: Lightnet.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c Lightnet.cpp

LightnetTAP.o: LightnetTAP.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetTAP.cpp

LightnetLIRC.o: LightnetLIRC.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetLIRC.cpp

LightnetD.o: LightnetD.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetD.cpp

Tester.o: Tester.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c Tester.cpp

lightnetd: Lightnet.o LightnetTAP.o LightnetLIRC.o LightnetD.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTAP.o LightnetLIRC.o LightnetD.o -o lightnetd

tester: Lightnet.o LightnetTAP.o LightnetLIRC.o Tester.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTAP.o LightnetLIRC.o Tester.o -o tester

profiler: Profiler.cpp
	$(CXX) $(CXXFlags) Profiler.cpp -o profiler


clean:
	rm *.o lightnetd tester profiler

install: lightnetd tester profiler
	mkdir /opt/lightnet
	mkdir /opt/lightnet/bin
	mkdir /opt/lightnet/doc
	sudo cp -r doc /opt/lightnet/doc
	sudo cp tester /opt/lightnet/bin
	sudo cp profiler /opt/lightnet/bin
	sudo cp lightnetd /opt/lightnet/bin
	sudo cp tap.sh /opt/lightnet/bin
	sudo cp arp.sh /opt/lightnet/bin
	sudo cp conf/lightnet.json /opt/lightnet/lightnet.json
	sudo cp conf/config.txt /boot/config.txt
	sudo cp conf/cmdline.txt /boot/cmdline.txt
	sudo cp conf/olsrd.conf /etc/olsrd/olsrd.conf
	sudo cp conf/dhcpd.conf /etc/dhcp/dhcpd.conf
	sudo cp conf/interfaces /etc/network/interfaces
	sudo cp conf/isc-dhcp-server /etc/default/isc-dhcp-server
	sudo cp conf/lightnetd /etc/init.d/lightnetd
	sudo chmod 744 /etc/init.d/lightnetd
