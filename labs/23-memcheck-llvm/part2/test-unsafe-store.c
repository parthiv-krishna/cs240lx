#include <stdio.h>
#include <stdlib.h>

void test(void){
    uint8_t * p = malloc(8);
    p[2] = 0;
    free(p);
    printf("Trying unsafe store now...\n");
    p[1] = 1;  
}

int main() {
    test();
    return 0;
}
