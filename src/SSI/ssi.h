#ifndef SSI_H
#define SSI_H

/*
   _____            __                    _____                 _              ____      __            ____
  / ___/__  _______/ /____  ____ ___     / ___/___  ______   __(_)_______     /  _/___  / /____  _____/ __/___ _________
  \__ \/ / / / ___/ __/ _ \/ __ `__ \    \__ \/ _ \/ ___/ | / / / ___/ _ \    / // __ \/ __/ _ \/ ___/ /_/ __ `/ ___/ _ \
 ___/ / /_/ (__  ) /_/  __/ / / / / /   ___/ /  __/ /   | |/ / / /__/  __/  _/ // / / / /_/  __/ /  / __/ /_/ / /__/  __/
/____/\__, /____/\__/\___/_/ /_/ /_/   /____/\___/_/    |___/_/\___/\___/  /___/_/ /_/\__/\___/_/  /_/  \__,_/\___/\___/
     /____/
*/

#include <mikabooq.h>

/* SSI's thread defined in ssi.c */
extern struct tcb_t *SSI;

/* SSI target function */
void ssi(void);

/* Returns a pointer to the ssi thread already initialized,
   extept for processor state */
struct tcb_t *ssi_thread_init(void);

#endif
