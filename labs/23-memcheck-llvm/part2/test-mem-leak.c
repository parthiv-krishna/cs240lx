#include <stdio.h>
#include <stdlib.h>

void test(void){
    u_int8_t * p = malloc(8);
    free(p);
    p = malloc(8);
}

int main() {
    test();
    return 0;
}
