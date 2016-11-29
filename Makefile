CXXFlags = -std=c++11 -lpthread
CXX = g++

all: lightnetd

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

Profiler.o: Profiler.cpp Lightnet.h
	$(CXX) $(CXXFlags) -c Profiler.cpp

lightnetd: Lightnet.o LightnetTAP.o LightnetLIRC.o LightnetD.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTAP.o LightnetLIRC.o LightnetD.o -o lightnetd

tester: Lightnet.o LightnetTAP.o LightnetLIRC.o Tester.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTAP.o LightnetLIRC.o Tester.o -o tester

profiler: Lightnet.o LightnetTAP.o LightnetLIRC.o Profiler.o
	$(CXX) $(CXXFlags) Lightnet.o LightnetTAP.o LightnetLIRC.o Profiler.o -o profiler

clean:
	rm *.o lightnetd tester profiler
