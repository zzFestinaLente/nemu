#include "cpu/exec/template-start.h"

#define instr jle

static void do_execute() {
    print_asm("jle %x", cpu.eip + 1 + DATA_BYTE + (DATA_TYPE_S)op_src -> val);
    if ((cpu.eflags.SF != cpu.eflags.OF)||cpu.eflags.ZF) {
        cpu.eip = cpu.eip + (DATA_TYPE_S)op_src -> val;
    }
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"