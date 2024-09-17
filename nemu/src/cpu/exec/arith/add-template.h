#include "cpu/exec/template-start.h"

#define instr add

static void do_execute() {    
    DATA_TYPE_S result = op_src -> val + op_dest -> val;
    OPERAND_W(op_dest, result);
    update_eflags_pf_zf_sf(result);
    int length = (DATA_BYTE << 3) - 1;
	cpu.eflags.CF = (result < op_dest -> val);
	cpu.eflags.OF = ((op_dest -> val >> length) != (op_src -> val >> length) && (op_src -> val >> length) == cpu.eflags.SF);
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