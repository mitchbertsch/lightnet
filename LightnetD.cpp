#include "Lightnet.h"
int main(int argc, char *argv[])
{
  Lightnet l;
  l.init_tap(0x01);
  cerr << "hit\n";
  l.init_lirc("/dev/lirc0");
  cerr << "hit\n";
  l.run();
}
