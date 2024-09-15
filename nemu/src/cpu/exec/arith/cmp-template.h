#include "cpu/exec/template-start.h"

#define instr cmp

// 封装标志位更新的宏
#define UPDATE_PF(cha) do { \
    cha ^= cha >> 4; \
    cha ^= cha >> 2; \
    cha ^= cha >> 1; \
    cpu.eflags.PF = !(cha & 1); \
} while(0)

#define UPDATE_FLAGS(cha, dest, src, len) do { \
    cpu.eflags.ZF = !(cha); \
    cpu.eflags.SF = (cha) >> (len); \
    cpu.eflags.CF = (dest) < (src); \
    int s1 = (dest) >> (len); \
    int s2 = (src) >> (len); \
    cpu.eflags.OF = (s1 != s2 && s2 == cpu.eflags.SF); \
    UPDATE_PF(cha); \
} while(0)

static void do_execute() {
    DATA_TYPE_S cha = (DATA_TYPE_S)(op_dest->val) - (DATA_TYPE_S)(op_src->val);
    int len = (DATA_BYTE << 3) - 1;
    UPDATE_FLAGS(cha, op_dest->val, op_src->val, len);
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
