#include <stdio.h>
#include <stdlib.h>

void test(void){
    uint8_t * p = malloc(8);
    p[1] = 8;
    uint8_t num = p[1];
    free(p);
    p =  malloc(8);
    free(p);
    printf("Should have run without any errors...\n");
}

int main() {
    test();
    return 0;
}
