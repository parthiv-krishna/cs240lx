#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void test(void){
    uint8_t * p = malloc(8);
    printf("Trying unsafe load now...\n");
    uint8_t num = p[10];
    free(p);
}

int main() {
    test();
    return 0;
}
