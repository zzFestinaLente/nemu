#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_, SUFFIX)) {
	cpu.eip = MEM_R(reg_l(R_ESP)) - 1;
    reg_l(R_ESP) += DATA_BYTE;
    //if(DATA_BYTE == 2) {        cpu.eip &= 0xffff;    }
    print_asm("ret");
    return 1;
}

make_helper(concat(ret_i_, SUFFIX)) {
	uint32_t num = instr_fetch(cpu.eip + 1, 2);
    cpu.eip = MEM_R(reg_l(R_ESP)) - 3;
    //if(DATA_BYTE == 2){		cpu.eip &= 0xffff;	}
    reg_l(R_ESP) += num;
    print_asm("ret 0x%x", num);
    return 3;
}

#include "cpu/exec/template-end.h"