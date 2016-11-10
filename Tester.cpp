#include "Lightnet.h"
int main(int argc, char *argv[])
{
  Lightnet l;
//  l.init_tap(0x01);
//  l.taps[0]->run();
  //cerr << "hit\n";
  l.init_lirc("/dev/lirc0");
  cerr << "hit\n";
  l.lircs[0]->run();
}
