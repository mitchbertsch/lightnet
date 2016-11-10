CXXFlags = -std=c++11 -lpthread
CXX = g++

all: lightnetd

Lightnet.o: Lightnet.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c Lightnet.cpp

LightnetTap.o: LightnetTap.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetTap.cpp

LightnetLIRC.o: LightnetLIRC.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetLIRC.cpp

LightnetD.o: LightnetD.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c LightnetD.cpp

Tester.o: Tester.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c Tester.cpp

lightnetd: Lightnet.o LightnetTap.o LightnetLIRC.o LightnetD.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTap.o LightnetLIRC.o LightnetD.o -o lightnetd

tester: Lightnet.o LightnetTap.o LightnetLIRC.o Tester.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTap.o LightnetLIRC.o Tester.o -o tester

clean:
	rm *.o lightnetd tester
