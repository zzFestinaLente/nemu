#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    //EIP ← EIP + SignExtend(rel8/16/32);
    //IF OperandSize = 16
    //THEN EIP ← EIP AND 0000FFFFH;
    DATA_TYPE_S imm = op_src -> val;
    print_asm("je %x", cpu.eip + 1 + DATA_BYTE + imm);
    if (cpu.eflags.ZF == 1) {
        cpu.eip = cpu.eip + imm;
    }
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"