#include "Lightnet.h"
int main(int argc, char *argv[])
{
  Lightnet l;
  //l.init(0x01,"/dev/lirc0");
  l.init_tap(0x01);
  l.init_lirc("/dev/lirc0");
  l.run();
}
