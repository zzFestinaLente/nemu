#include "cpu/exec/template-start.h"

#define instr push

// static void do_execute() { 
//     if(DATA_BYTE == 1) {
//         op_src -> val = (int8_t)op_src -> val;
// 	}
// 	if(DATA_BYTE==2){
// 		 reg_l(R_ESP) -= DATA_BYTE;
// 	}
// 	else{
// 		 reg_l(R_ESP) -= 4;
// 	}
//    // reg_l(R_ESP) -= DATA_BYTE;
//     swaddr_write(reg_l(R_ESP), 4, op_src -> val);
//     print_asm_template1();
//     //print_asm("push %x", reg_l(R_EAX));
// }

static void do_execute(){
    cpu.esp -= 4;  //栈顶地址减4来存放数据
    swaddr_write(cpu.esp, 4, op_src->val);  //写入目标数据
    print_asm_template1();
}

make_instr_helper(i)
#if DATA_BYTE == 2  || DATA_BYTE == 4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"