#include "cpu/exec/template-start.h"

#define instr push

static void do_execute(){
	//栈顶地址减4
    cpu.esp -= 4;  
	//写入
    swaddr_write(cpu.esp, 4, op_src->val);  
    print_asm_template1();
}

make_instr_helper(i)
#if DATA_BYTE == 2  || DATA_BYTE == 4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"