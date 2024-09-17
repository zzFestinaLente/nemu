#include "cpu/exec/template-start.h"

#define instr jne

static void do_execute() {
    print_asm("jne %x", cpu.eip + 1 + DATA_BYTE + (DATA_TYPE_S)op_src -> val);
    if (!cpu.eflags.ZF) {
        cpu.eip = cpu.eip + (DATA_TYPE_S)op_src -> val;
    }
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"