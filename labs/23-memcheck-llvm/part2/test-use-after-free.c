#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void test(void){
    uint8_t * p = malloc(8);
    free(p);
    p = malloc(8);
    free(p);
    printf("Trying to use after free now...\n");
    p[1] = 0;
}

int main() {
    test();
    return 0;
}
