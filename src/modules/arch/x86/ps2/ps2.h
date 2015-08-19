#define PS2_BUS                         0x0003
#define PS2_KEYBOARD                    0x0001
#define PS2_MOUSE                       0x0002

unsigned char ps2_getdata(void);
unsigned short ps2_getirq(unsigned int id);
void ps2_enable(unsigned int id);
void ps2_enableinterrupt(unsigned int id);
void ps2_reset(unsigned int id);
void ps2_identify(unsigned int id);
void ps2_enablescanning(unsigned int id);
void ps2_disablescanning(unsigned int id);
void ps2_default(unsigned int id);
