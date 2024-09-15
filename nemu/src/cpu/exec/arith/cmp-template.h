#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
    DATA_TYPE_S cha = (DATA_TYPE_S)(op_dest -> val) - (DATA_TYPE_S)(op_src -> val);
    //printf("------------------------------------%x, %x, %x\n", op_src -> val, op_dest -> val, cha);
    int len = (DATA_BYTE << 3) - 1;
    //update_eflags_pf_zf_sf(cha);
    cpu.eflags.ZF = !cha;
    cpu.eflags.SF = cha >> len;
	cpu.eflags.CF = op_dest -> val < op_src -> val;
    int s1 = op_dest -> val >> len;
	int s2 = op_src -> val >> len;
	cpu.eflags.OF = (s1 != s2 && s2 == cpu.eflags.SF);
    cha ^= cha >> 4;
	cha ^= cha >> 2;
	cha ^= cha >> 1;
	cpu.eflags.PF = !(cha & 1);
    print_asm_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"