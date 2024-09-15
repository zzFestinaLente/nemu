#include "cpu/exec/template-start.h"

#define instr setne

static void do_execute() {
    //DATA_TYPE_S imm = op_src -> val;
    if (cpu.eflags.ZF == 0) {
        //cpu.eip = cpu.eip + imm;
        OPERAND_W (op_src, 1);
    }
    else OPERAND_W (op_src, 0);//???
    print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"