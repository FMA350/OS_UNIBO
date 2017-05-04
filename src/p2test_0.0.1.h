#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>

#include "const.h"
#include "listx.h"
#include "mikabooq.h"
#include "nucleus.h"
#include <stdint.h>

#define QPAGE FRAME_SIZE
#define TERM0ADDR               0x24C


static void ttyprintstring(devaddr device, char *s);
void tty0out_thread(void);
static inline void tty0print(char *s);
static inline void panic(char *s);
static inline void CSIN();
void cs_thread(void);

void p2(),p3(),p4(),p5(),p6(),p7(),p8();
struct tcb_t *testt,*p2t,*p3t,*p4t,*p5t,*p6t,*p7t,*p8t;
void test(void);
