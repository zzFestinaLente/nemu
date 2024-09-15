#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {

    int len = concat(decode_i_, SUFFIX)(cpu.eip + 1);
    reg_l(R_ESP) -= DATA_BYTE;
    swaddr_write(reg_l(R_ESP), 4, cpu.eip + len + 1);
    DATA_TYPE_S mid = op_src -> val;
    print_asm("call %x", cpu.eip + 1 + len + mid);
    cpu.eip += (DATA_TYPE_S)op_src -> val;
    return len + 1;
}

make_helper(concat(call_rm_, SUFFIX)) { 
    int len = concat(decode_rm_, SUFFIX)(cpu.eip + 1);
    reg_l(R_ESP) -= DATA_BYTE;
    swaddr_write(reg_l(R_ESP), 4, cpu.eip + len + 1);
    print_asm("call %x", (DATA_TYPE_S)op_src -> val);
    cpu.eip = (DATA_TYPE_S)op_src -> val - len - 1;
    return len + 1;
}

#include "cpu/exec/template-end.h"