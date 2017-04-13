
/*
EXCEPTION VECTOR
Fast Interrupt Request    0x0000.001C
Interrupt Request         0x0000.0018
reserved/unused           0x0000.0014
Data Abort                0x0000.0010
Prefetch Abort            0x0000.000C
Software Interrupt        0x0000.0008
Undefined Instruction     0x0000.0004
Reset                     0x0000.0000

the PC jumps to these addresses. If the the system
is correctly set up, a branch instruction will lead the execution
to the correct handler.
*/

/*

void SoftwareInterrupt(unsigned int SystemCAllNumber, unsigned int a1, unsigned int a2, unsigned int a3){
    //0x0000.0008
    if(SystemCAllNumber == SYS_SEND)
        int err = send(a1, a2); //TODO to be defined

    else if (SystemCAllNumber == SYS_RECV)
        recv (a1, a2);

    else return //TODO: throws an excepion.
}

*/
