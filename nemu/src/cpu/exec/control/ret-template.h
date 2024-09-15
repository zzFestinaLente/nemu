#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_, SUFFIX)) {
    cpu.eip = swaddr_read(cpu.esp, 4) - 1;
    reg_l(R_ESP) += 4;
    print_asm("ret");
    return 1;
}

make_helper(concat(ret_i_, SUFFIX)) {
    cpu.eip = MEM_R(reg_l(R_ESP)) - 3;
    reg_l(R_ESP) += (uint32_t)instr_fetch(cpu.eip + 1, 2);
    print_asm("ret 0x%x", (uint32_t)instr_fetch(cpu.eip + 1, 2));
    return 3;
}

#include "cpu/exec/template-end.h"