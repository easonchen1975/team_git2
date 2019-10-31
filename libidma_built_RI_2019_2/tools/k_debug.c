
#include <stdio.h>
#include <stdint.h>
//#include <string.h>

//#include <xtensa/idma.h>
//#include "idma_tests.h"


#include "xtensa_ops.h"
void K_dump(){

    uint32_t exccause, epc1, epc2, epc3, excvaddr, depc, excsave1, excsave2, interrupt;
    uint32_t intenable, pc, ps, sp;
    uint32_t excinfo[8];

    SP(sp);
    //PC(pc);
    RSR(exccause, exccause);
    RSR(epc1, epc1);
    RSR(epc2, epc2);
    RSR(epc3, epc3);
    RSR(excvaddr, excvaddr);
    RSR(depc, depc);
    RSR(excsave1, excsave1);
    RSR(interrupt, interrupt);
    RSR(intenable, intenable);
    RSR(ps, ps);
    RSR(excsave2,excsave2);
    printf("Fatal interrupt (%d): \n", (int)exccause);
    printf("%s=0x%08x\n", "sp", sp);
    printf("%s=0x%08x\n", "epc1", epc1);
    printf("%s=0x%08x\n", "epc2", epc2);
    printf("%s=0x%08x\n", "epc3", epc3);
    printf("%s=0x%08x\n", "excvaddr", excvaddr);
    printf("%s=0x%08x\n", "depc", depc);
    printf("%s=0x%08x\n", "excsave1", excsave1);
    printf("%s=0x%08x\n", "excsave2", excsave2);
    printf("%s=0x%08x\n", "intenable", intenable);
    printf("%s=0x%08x\n", "interrupt", interrupt);
    printf("%s=0x%08x\n", "ps", ps);

}

void K_dump_exc(){

    uint32_t exccause, epc1, epc2, epc3, excvaddr, depc, excsave1, excsave2, interrupt;
    uint32_t intenable, pc, ps;
    uint32_t excinfo[8];

    WSR(intenable, 0);
    RSR(exccause, exccause);
    RSR(epc1, epc1);
    RSR(epc2, epc2);
    RSR(epc3, epc3);
    RSR(excvaddr, excvaddr);
    RSR(depc, depc);
    RSR(excsave1, excsave1);
    RSR(interrupt, interrupt);
    RSR(intenable, intenable);
    RSR(ps, ps);
    RSR(excsave2,excsave2);
    printf("Fatal exception (%d): \n", (int)exccause);
    printf("%s=0x%08x\n", "epc1", epc1);
    printf("%s=0x%08x\n", "epc2", epc2);
    printf("%s=0x%08x\n", "epc3", epc3);
    printf("%s=0x%08x\n", "excvaddr", excvaddr);
    printf("%s=0x%08x\n", "depc", depc);
    printf("%s=0x%08x\n", "excsave1", excsave1);
    printf("%s=0x%08x\n", "excsave2", excsave2);
    printf("%s=0x%08x\n", "intenable", intenable);
    printf("%s=0x%08x\n", "interrupt", interrupt);
    printf("%s=0x%08x\n", "ps", ps);

}
