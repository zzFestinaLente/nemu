#include "cpu/exec/template-start.h"

#define instr ja

static void do_execute() {
    print_asm("ja %x", cpu.eip + 1 + DATA_BYTE + (DATA_TYPE_S)op_src -> val);
    if (!cpu.eflags.CF && !cpu.eflags.ZF) cpu.eip = cpu.eip + (DATA_TYPE_S)op_src -> val;
}

make_instr_helper(i)
make_helper(ja_i_b); 

#include "cpu/exec/template-end.h"