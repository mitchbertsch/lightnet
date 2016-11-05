CXXFlags = -std=c++11 -lpthread
CXX = g++

all: lightnetd

Lightnet.o: Lightnet.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c Lightnet.cpp

LightnetTap.o: LightnetTap.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetTap.cpp

LightnetLIRC.o: LightnetLIRC.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetLIRC.cpp

lightnetd: Lightnet.o LightnetTap.o LightnetLIRC.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTap.o LightnetLIRC.o -o lightnetd
clean:
	rm *.o lightnetd
