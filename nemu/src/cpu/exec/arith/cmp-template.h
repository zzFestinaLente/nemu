#include "cpu/exec/template-start.h"

#define instr cmp

// 封装标志位更新的宏
#define UPDATE_PF(res) do { \
    res ^= res >> 4; \
    res ^= res >> 2; \
    res ^= res >> 1; \
    cpu.eflags.PF = !(res & 1); \
} while(0)

#define UPDATE_FLAGS(res, dest, src, len) do { \
    cpu.eflags.ZF = !(res); \
    cpu.eflags.SF = (res) >> (len); \
    cpu.eflags.CF = (dest) < (src); \
    int s1 = (dest) >> (len); \
    int s2 = (src) >> (len); \
    cpu.eflags.OF = (s1 != s2 && s2 == cpu.eflags.SF); \
    UPDATE_PF(res); \
} while(0)

static void do_execute() {
    DATA_TYPE_S res = (DATA_TYPE_S)(op_dest->val) - (DATA_TYPE_S)(op_src->val);
    int len = (DATA_BYTE << 3) - 1;
    UPDATE_FLAGS(res, op_dest->val, op_src->val, len);
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
