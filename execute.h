/*-*- Mode: C; c-basic-offset: 8 -*-*/

#ifndef fooexecutehfoo
#define fooexecutehfoo

typedef struct ExecStatus ExecStatus;
typedef struct ExecCommand ExecCommand;
typedef struct ExecContext ExecContext;

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/capability.h>
#include <stdbool.h>
#include <stdio.h>
#include <sched.h>

#include "list.h"
#include "util.h"

/* Abstract namespace! */
#define LOGGER_SOCKET "/systemd/logger"

typedef enum ExecOutput {
        EXEC_OUTPUT_CONSOLE,
        EXEC_OUTPUT_NULL,
        EXEC_OUTPUT_SYSLOG,
        EXEC_OUTPUT_KERNEL,
        _EXEC_OUTPUT_MAX,
        _EXEC_OUTPUT_INVALID = -1
} ExecOutput;

typedef enum ExecInput {
        EXEC_INPUT_NULL,
        EXEC_INPUT_CONSOLE,
        _EXEC_INPUT_MAX,
        _EXEC_INPUT_INVALID = -1
} ExecInput;

struct ExecStatus {
        pid_t pid;
        usec_t timestamp;
        int code;     /* as in siginfo_t::si_code */
        int status;   /* as in sigingo_t::si_status */
};

struct ExecCommand {
        char *path;
        char **argv;
        ExecStatus exec_status;
        LIST_FIELDS(ExecCommand, command); /* useful for chaining commands */
};

struct ExecContext {
        char **environment;
        mode_t umask;
        struct rlimit *rlimit[RLIMIT_NLIMITS];
        char *working_directory, *root_directory;
        int oom_adjust;
        int nice;
        int ioprio;
        int cpu_sched_policy;
        int cpu_sched_priority;
        cpu_set_t cpu_affinity;
        unsigned long timer_slack_ns;

        bool oom_adjust_set:1;
        bool nice_set:1;
        bool ioprio_set:1;
        bool cpu_sched_set:1;
        bool cpu_affinity_set:1;
        bool timer_slack_ns_set:1;

        ExecInput input;
        ExecOutput output;
        int syslog_priority;
        char *syslog_identifier;

        /* FIXME: all privs related settings need to be enforced */
        cap_t capabilities;
        int secure_bits;
        uint64_t capability_bounding_set_drop;

        /* Since resolving these names might might involve socket
         * connections and we don't want to deadlock ourselves these
         * names are resolved on execution only and in the child
         * process. */
        char *user;
        char *group;
        char **supplementary_groups;
};

typedef enum ExitStatus {
        /* EXIT_SUCCESS defined by libc */
        /* EXIT_FAILURE defined by libc */
        EXIT_INVALIDARGUMENT = 2,
        EXIT_NOTIMPLEMENTED = 3,
        EXIT_NOPERMISSION = 4,
        EXIT_NOTINSTALLED = 5,
        EXIT_NOTCONFIGURED = 6,
        EXIT_NOTRUNNING = 7,

        /* The LSB suggests that error codes >= 200 are "reserved". We
         * use them here under the assumption that they hence are
         * unused by init scripts.
         * c->
         *
         * http://refspecs.freestandards.org/LSB_3.1.0/LSB-Core-generic/LSB-Core-generic/iniscrptact.html */

        EXIT_CHDIR = 200,
        EXIT_NICE,
        EXIT_FDS,
        EXIT_EXEC,
        EXIT_MEMORY,
        EXIT_LIMITS,
        EXIT_OOM_ADJUST,
        EXIT_SIGNAL_MASK,
        EXIT_INPUT,
        EXIT_OUTPUT,
        EXIT_CHROOT,
        EXIT_PGID,
        EXIT_IOPRIO,
        EXIT_TIMERSLACK,
        EXIT_SECUREBITS,
        EXIT_SETSCHEDULER,
        EXIT_CPUAFFINITY
} ExitStatus;

int exec_spawn(const ExecCommand *command, const ExecContext *context, int *fds, unsigned n_fds, pid_t *ret);

void exec_command_free_list(ExecCommand *c);
void exec_command_free_array(ExecCommand **c, unsigned n);

char *exec_command_line(ExecCommand *c);
void exec_command_dump(ExecCommand *c, FILE *f, const char *prefix);
void exec_command_dump_list(ExecCommand *c, FILE *f, const char *prefix);

void exec_context_init(ExecContext *c);
void exec_context_done(ExecContext *c);
void exec_context_dump(ExecContext *c, FILE* f, const char *prefix);

void exec_status_fill(ExecStatus *s, pid_t pid, int code, int status);

const char* exec_output_to_string(ExecOutput i);
int exec_output_from_string(const char *s);

const char* exec_input_to_string(ExecInput i);
int exec_input_from_string(const char *s);

#endif
