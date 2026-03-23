#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main() { 
    void *p = malloc(9999ULL * 1024 * 1024 * 1024);
    if(p) {
        memset(p, 1, 9999ULL * 1024 * 1024 * 1024);
    }
    return 0; 
}
