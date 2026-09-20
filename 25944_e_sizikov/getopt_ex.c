#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <errno.h>
#include <string.h>


#define MAX_OPT 256

extern char *optarg;
extern char **environ;

typedef struct {
    int opt;
    char *optarg;
} Opt_inf;

int main(int argc, char *argv[]) {
    Opt_inf opts[MAX_OPT];
    int opt_count = 0;
    int c;

    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (opt_count < MAX_OPT) {
            opts[opt_count].opt = c;
            opts[opt_count].optarg = optarg;
            opt_count++;
        } else {
            fprintf(stderr, "Maximum options is limited: (%d)\n", MAX_OPT);
            break;
        }
    }

    for (int i = opt_count - 1; i >= 0; i--) {
        switch (opts[i].opt) {
            case 'i': {
                printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;
            }
            case 's': {
                if (setpgid(0, 0) == -1) {
                    perror("Error of executing setpgid");
                } else {
                    printf("New PGID: %d\n", getpgrp());
                }
                break;
            }
            case 'p': {
                printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
                break;
            }
            case 'u': {
                long lim = ulimit(UL_GETFSIZE, 0);
                if (lim == -1) {
                    perror("Error with ulimit");
                } else {
                    printf("Ulimit: %ld\n", lim);
                }
                break;
            }
            case 'U': {
                char *endptr;
                errno = 0;
                long new_lim = strtol(opts[i].optarg, &endptr, 10);

                if (*endptr != '\0' || endptr == opts[i].optarg || errno != 0 || new_lim < 0) {
                   fprintf(stderr, "Invalid value: %s\n", opts[i].optarg);
                   break;
                }

                if (ulimit(UL_SETFSIZE, new_lim) == -1) {
                    perror("Error with ulimit");
                } else {
                    printf("New Ulimit: %ld\n", ulimit(UL_GETFSIZE, 0));
                }
                break;
            }
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("Error with getrlimit");
                } else {
                    printf("Max size core: %ld\n", (long)rl.rlim_cur);
                }
                break;
            }
            case 'C': {
                char *endptr;
                errno = 0;
                long size = strtol(opts[i].optarg, &endptr, 10);

                if (*endptr != '\0' || endptr == opts[i].optarg || errno != 0 || size < 0) {
                    fprintf(stderr, "Invalid core size value: '%s'\n", opts[i].optarg);
                    break;
                }

                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1 || size < 0) {
                    perror("Error with getrlimit");
                    break;
                }
                rl.rlim_cur = size;
                if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("Error with setrlimit");
                } else {
                    printf("New size core: %ld\n", size);
                }
                break;
            }
            case 'd': {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current dir: %s\n", cwd);
                }
                break;
            }
            case 'v': {
                printf("Enviromental variables:\n");
                for (char **env = environ; *env != NULL; env++) {
                    printf("  %s\n", *env);
                }
                break;
            }
            case 'V': {
                if (putenv(opts[i].optarg) != 0) {
                    perror("Error with putenv");
                } else {
                    printf("Added/changed env variable: %s\n", opts[i].optarg);
                }
                break;
            }
            default:
                break;
        }
    }

    return 0;
}
