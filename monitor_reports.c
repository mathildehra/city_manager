#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define MONITOR_PID_FILE ".monitor_pid"

static void emit(const char *type, const char *text){
    time_t now = time(NULL);
    char ts[32];
    struct tm *t = localtime(&now);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);
    printf("%s|[%s] %s\n", type, ts, text);
    fflush(stdout);
}

static volatile sig_atomic_t got_sigusr1 = 0;
static volatile sig_atomic_t got_sigint  = 0;

static void handler_sigusr1(int sig) { (void)sig; got_sigusr1 = 1; }
static void handler_sigint(int sig)  { (void)sig; got_sigint  = 1; }

static pid_t read_existing_pid(void){
    int fd = open(MONITOR_PID_FILE, O_RDONLY);
    if (fd < 0) return -1;

    char buf[32];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return -1;

    buf[n] = '\0';
    char *end;
    long pid = strtol(buf, &end, 10);
    if (pid <= 0) return -1;
    return (pid_t)pid;
}

static int process_alive(pid_t pid){
    if(kill(pid,0)==0){
        return 1;
    } else return 0;
}

int main(void){
    setlinebuf(stdout);

    pid_t existing = read_existing_pid();
    if (existing > 0 && process_alive(existing)) {
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "Another monitor is already running with PID %ld",
                 (long)existing);
        emit("ERROR", msg);
        emit("END",   "monitor exiting without starting");
        exit(1);
    }

    pid_t pid = getpid();
    int fd = open(MONITOR_PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        emit("ERROR", "Could not write .monitor_pid");
        emit("END",   "monitor exiting");
        exit(1);
    }
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%ld\n", (long)pid);
    write(fd, buf, (size_t)len);
    close(fd);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = handler_sigusr1;
    sa.sa_flags   = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        emit("ERROR", "sigaction SIGUSR1 failed");
        emit("END",   "monitor exiting");
        unlink(MONITOR_PID_FILE);
        exit(1);
    }

    sa.sa_handler = handler_sigint;
    sa.sa_flags   = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        emit("ERROR", "sigaction SIGINT failed");
        emit("END",   "monitor exiting");
        unlink(MONITOR_PID_FILE);
        exit(1);
    }

    char startmsg[64];
    snprintf(startmsg, sizeof(startmsg),
             "monitor_reports started (PID %ld)", (long)getpid());
    emit("INFO", startmsg);

    while (!got_sigint) {
        pause();

        if (got_sigusr1) {
            got_sigusr1 = 0;
            emit("EVENT", "SIGUSR1 received — a new report has been added");
        }
    }

    emit("INFO", "SIGINT received — shutting down");
    emit("END",  "monitor_reports stopped");

    unlink(MONITOR_PID_FILE);
    return 0;
}
