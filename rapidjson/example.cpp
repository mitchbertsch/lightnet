#include "filereadstream.h" // will include "rapidjson/rapidjson.h"
#include "document.h"
#include <iostream>
using namespace rapidjson;
using namespace std;

int main(){

FILE* fp = fopen("config.json", "r");
char readBuffer[65536];
FileReadStream  s(fp, readBuffer, sizeof(readBuffer));
Document d;
d.ParseStream(s);
fclose(fp);

const int pulseZeroMin = d["pulseZeroMin"].GetInt();
cout<<pulseZeroMin<<"\n";

const Value& pulseZero = d["pulseZero"];
cout<<pulseZero[0].GetString()<<" "<<pulseZero[1].GetString()<<" ";
cout<<pulseZero[2].GetString()<<" "<<pulseZero[3].GetString()<<"\n";

}
