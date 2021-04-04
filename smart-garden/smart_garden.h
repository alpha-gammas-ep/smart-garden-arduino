typedef struct  plant {
  int light_interval;
  int water_interval;
  unsigned long last_watered;
  int water_volume;
} plant;

unsigned long get_epoch();
boolean load_plant(int num);
unsigned long sendNTPpacket(IPAddress& address);
void long_to_string(char* buf, long l);
unsigned long string_to_long(char* c);
