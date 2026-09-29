#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <errno.h>
#include <limits.h>

#define MAX_OPT 256

extern char *optarg;
extern char **environ;

typedef struct {
    int opt;
    char *optarg;
} Opt_inf;

int parse_nonneg_long(const char *s, long *out)
{
    char *endptr;
    long v;

    errno = 0;
    v = strtol(s, &endptr, 10);
    if (endptr == s || *endptr != '\0' || errno != 0 || v < 0)
        return -1;
    *out = v;
    return 0;
}

long get_core_limit_ulimit(void)
{
    struct rlimit rl;

    if (getrlimit(RLIMIT_CORE, &rl) == -1)
        return -1;
    if (rl.rlim_cur == RLIM_INFINITY)
        return -2;
    return (long)(rl.rlim_cur / 1024);
}

// after -U -u prints correct
long nproc_override = -1;

long get_nproc_limit(void)
{
    if (nproc_override >= 0)
        return nproc_override;
    return sysconf(_SC_CHILD_MAX);
}

void print_nproc(void)
{
    long n = get_nproc_limit();

    if (n < 0)
        perror("ulimit -u");
    else
        printf("%ld\n", n);
}

void print_core(void){
    long n = get_core_limit_ulimit();

    if (n == -2)
        printf("unlimited\n");
    else if (n < 0)
        perror("ulimit -c");
    else
        printf("%ld\n", n);
}

int main(int argc, char *argv[])
{
    Opt_inf opts[MAX_OPT];
    int opt_count = 0;
    int c;
    int i;

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

    for (i = opt_count - 1; i >= 0; i--) {
        switch (opts[i].opt) {
            case 'i':
                printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1) {
                    perror("Error of executing setpgid");
                } else {
                    printf("New PGID: %d\n", (int)getpgrp());
                }
                break;

            case 'p':
                printf("PID: %d, PPID: %d, PGID: %d\n",
                       (int)getpid(), (int)getppid(), (int)getpgrp());
                break;

            case 'u':
                print_nproc();
                break;

            case 'U': {
                long newlimit;

                if (parse_nonneg_long(opts[i].optarg, &newlimit) == -1) {
                    fprintf(stderr, "Invalid value: %s\n", opts[i].optarg);
                    break;
                }
                nproc_override = newlimit;
                printf("%ld\n", newlimit);
                break;
            }

            case 'c':
                print_core();
                break;

            case 'C': {
                struct rlimit rl;
                long kb;

                if (parse_nonneg_long(opts[i].optarg, &kb) == -1) {
                    fprintf(stderr, "Invalid core size value: '%s'\n", opts[i].optarg);
                    break;
                }
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("getrlimit(RLIMIT_CORE)");
                    break;
                }
                
                rl.rlim_cur = (rlim_t)(kb * 1024);
                if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("setrlimit(RLIMIT_CORE)");
                } else {
                    printf("%ld\n", kb);
                }
                break;
            }

            case 'd': {
                char cwd[1024];

                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current dir: %s\n", cwd);
                } else {
                    perror("Error with getcwd");
                }
                break;
            }

            case 'v': {
                char **env;

                printf("Enviromental variables:\n");
                for (env = environ; *env != NULL; env++) {
                    printf("  %s\n", *env);
                }
                break;
            }

            case 'V':
                if (putenv(opts[i].optarg) != 0) {
                    perror("Error with putenv");
                } else {
                    printf("Added/changed env variable: %s\n", opts[i].optarg);
                }
                break;

            case '?':
            default:
                fprintf(stderr, "unknown option\n");
                break;
        }
    }

    return 0;
}
