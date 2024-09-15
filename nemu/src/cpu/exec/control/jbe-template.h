#include "cpu/exec/template-start.h"

#define instr jbe

static void do_execute() {
    print_asm("jbe %x", cpu.eip + 1 + DATA_BYTE + (DATA_TYPE_S)op_src -> val);
    if (cpu.eflags.ZF|| cpu.eflags.CF) {
        cpu.eip = cpu.eip + (DATA_TYPE_S)op_src -> val;
    }
}

make_instr_helper(i)
make_helper(jbe_i_b);

#include "cpu/exec/template-end.h"