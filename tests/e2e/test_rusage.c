#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        void *p = malloc(100ULL * 1024 * 1024); // 100 MB
        exit(0);
    } else {
        int status;
        struct rusage ru;
        wait4(pid, &status, 0, &ru);
        printf("ru_maxrss: %ld\n", ru.ru_maxrss);
    }
    return 0;
}
