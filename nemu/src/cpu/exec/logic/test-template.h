#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
    DATA_TYPE_S cha = (op_dest -> val) & (op_src -> val);
    update_eflags_pf_zf_sf(cha);
    //int len = (DATA_BYTE << 3) - 1;
	cpu.eflags.CF = 0;
	cpu.eflags.OF = 0;   
    print_asm_template2();
}

make_instr_helper(r2rm)
make_instr_helper(i2rm)
make_instr_helper(i2a)

#include "cpu/exec/template-end.h"