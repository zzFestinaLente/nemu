#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "add-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "add-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "add-template.h"
#undef DATA_BYTE

make_helper(add_r2rm_b);
make_helper_v(add_r2rm)
make_helper(add_rm2r_b);
make_helper_v(add_rm2r)
make_helper_v(add_si2rm)
make_helper(add_i2r_b);
make_helper_v(add_i2a)
make_helper(add_i2a_b);
make_helper(add_i2rm_b);
make_helper_v(add_i2rm)
