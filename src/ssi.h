/*
   _____            __                    _____                 _              ____      __            ____
  / ___/__  _______/ /____  ____ ___     / ___/___  ______   __(_)_______     /  _/___  / /____  _____/ __/___ _________
  \__ \/ / / / ___/ __/ _ \/ __ `__ \    \__ \/ _ \/ ___/ | / / / ___/ _ \    / // __ \/ __/ _ \/ ___/ /_/ __ `/ ___/ _ \
 ___/ / /_/ (__  ) /_/  __/ / / / / /   ___/ /  __/ /   | |/ / / /__/  __/  _/ // / / / /_/  __/ /  / __/ /_/ / /__/  __/
/____/\__, /____/\__/\___/_/ /_/ /_/   /____/\___/_/    |___/_/\___/\___/  /___/_/ /_/\__/\___/_/  /_/  \__,_/\___/\___/
     /____/
*/

#ifndef SSI_H
#define SSI_H

#include <mikabooq.h>


/* SSI daemon */
// void SSI();

// FIXME: should errorNumber belong to the struct tcb_t???
int errorNumber;
// TODO: is error number different from thread to thread???

/* Returns the ssi thread already initialized, extept for processor state */
struct tcb_t *ssi_thread_init();



#endif
