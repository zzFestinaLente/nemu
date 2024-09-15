#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    DATA_TYPE imm = op_src -> val;
    print_asm("je %x", cpu.eip + 1 + DATA_BYTE + (DATA_TYPE_S)imm);
    if (cpu.eflags.ZF == 1) {
        cpu.eip = cpu.eip + (DATA_TYPE_S)imm;
    }
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"