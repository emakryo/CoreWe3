#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "float_arith.h"

extern uint32_t fsin(uint32_t a);

int main() {
    float fa, fans;
    uint32_t ans;
    srand((unsigned) time(NULL));
    
    fa = 16.0;
    fans = sinf(fa);
    ans = fsin(ftoi(fa));
    printf("%08x %08x\n", ftoi(fans), ans);
    printf("%e %e\n", fans, itof(ans));
    
    return 0;
}
