#include "cpu/exec/template-start.h"

#define instr jl

static void do_execute() {
    print_asm("jl %x", cpu.eip + 1 + DATA_BYTE + (DATA_TYPE_S)op_src -> val);
    if (cpu.eflags.SF != cpu.eflags.OF) {
        cpu.eip = cpu.eip + (DATA_TYPE_S)op_src -> val;
    }
}

make_instr_helper(i)
make_helper(jl_i_b); 

#include "cpu/exec/template-end.h"