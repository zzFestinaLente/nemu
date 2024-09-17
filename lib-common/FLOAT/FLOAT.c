#include "FLOAT.h"

#include <stdint.h>
 
FLOAT F_mul_F(FLOAT a, FLOAT b) {
	int64_t scale = ((int64_t)a * (int64_t)b) >> 16;
	return scale;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    int sign = 1;
    FLOAT dividend, divisor, quotient, remainder;

    if (a < 0) { sign = -sign; dividend = -a; } else { dividend = a; }
    if (b < 0) { sign = -sign; divisor = -b; } else { divisor = b; }

    quotient = dividend / divisor;
    remainder = dividend % divisor;
	int i;
    for (i = 0; i < 16; i++) {
        remainder <<= 1;
        quotient <<= 1;
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient++;
        }
    }

    return sign * quotient;
}

FLOAT f2F(float a) {
    union {
        float f;
        int i;
    } converter = {a};

    int value = converter.i;
    int sign = value & 0x80000000;
    int exponent = ((value >> 23) & 0xff) - 150;
    int fraction = (value & 0x7fffff) | (value ? 0x800000 : 0);

    if (exponent < -16) {
        fraction >>= -16 - exponent;
    } else if (exponent > -16) {
        fraction <<= exponent + 16;
    }

    return sign ? -fraction : fraction;
}

FLOAT Fabs(FLOAT a) {
	return a>0?a:-a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {  //PA4中会用到，目前不用
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

