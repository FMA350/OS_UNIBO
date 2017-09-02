inline void setdevice(unsigned int devno, uintptr_t command){
    //tprint("setting transmitChar command\n");
    void *p = (void *) 0x0000024c + ((0x10)*devno);
    *((unsigned int *)p) = command;
    //tprint("\n..........completed transmitChar command\n");
}

inline unsigned int do_io_s(uintptr_t msgg, struct tcb_t* applic)
{
    //tprint("    do_io_s started\n");
/*    switch (req_field(msgg,1)) {
        case TERM0ADDR:   //il device e' un terminale */
        int empty = 1;
        int i;

        // the thread gets soft blocked
        soft_block_count++;

        //TODO: possiamo fare una lista per ogni terminale invece che array statico
        while (request[i].requester==NULL && i<8)
            i++;

        if (request[i].requester!=NULL)
            empty = 0;

        if(empty){
            setdevice(0,req_field(msgg,2));
            request[0].val = msgg;
            request[0].requester = applic;
        }
        //aggiorno -> (using device)
        else {
            i=0;
            while(request[i].requester!=NULL && i<8)
                i++; //cerco il primo buco libero per salvare il messaggio

            if (i==8)
                return -1; //se non ci sono piu spazi per salvare...
            else {
                request[i].val = msgg;
                request[i].requester = applic;
            }
        }
        /*break;
        case PRINTADDR:
        break;
        case NETADDR:
        break;
        case TAPEADDR:
        break;
        case DISKADDR:
        break;
        default: return -1;
    }*/
    //tprint("    do_io_s finished\n");
}
