#include <stdlib.h>
#include <stdio.h>
int main() { 
    void *p = malloc(9999ULL * 1024 * 1024 * 1024); 
    if(!p) {
        printf("Malloc failed\n");
        abort(); 
    }
    return 0; 
}
