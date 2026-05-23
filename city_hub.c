#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#define MONITOR_EXE   "./monitor_reports"
#define SCORER_EXE    "./district_scorer"
#define LINE_MAX_LEN  1024
#define MAX_DISTRICTS 64

static void hub_mon(void){
    int pipefd[2];
    if (pipe(pipefd) < 0) { perror("hub_mon: pipe"); exit(1); }

    pid_t mon_pid = fork();
    if (mon_pid < 0) { perror("hub_mon: fork monitor"); exit(1); }

    if (mon_pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("hub_mon: dup2"); exit(1);
        }
        close(pipefd[1]);
        execl(MONITOR_EXE, MONITOR_EXE, (char *)NULL);
        perror("hub_mon: execl monitor_reports");
        exit(1);
    }

    close(pipefd[1]);

    FILE *pipe_in = fdopen(pipefd[0], "r");
    if (!pipe_in) { perror("hub_mon: fdopen"); exit(1); }

    char line[LINE_MAX_LEN];
    while (fgets(line, sizeof(line), pipe_in) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';

        char *sep = strchr(line, '|');
        if (!sep) {
            printf("[monitor] %s\n", line);
            fflush(stdout);
            continue;
        }

        *sep = '\0';
        const char *type = line;
        const char *text = sep + 1;

        if      (strcmp(type, "INFO")  == 0) printf("[monitor INFO ] %s\n", text);
        else if (strcmp(type, "EVENT") == 0) printf("[monitor EVENT] %s\n", text);
        else if (strcmp(type, "ERROR") == 0) printf("[monitor ERROR] %s\n", text);
        else if (strcmp(type, "END")   == 0) {
            printf("[monitor END  ] %s\n", text);
            printf("[hub_mon] Monitor has ended. hub_mon exiting.\n");
            fflush(stdout);
            fclose(pipe_in);
            waitpid(mon_pid, NULL, 0);
            exit(0);
        } else {
            printf("[monitor ?????] %s|%s\n", type, text);
        }
        fflush(stdout);
    }

    printf("[hub_mon] Pipe closed unexpectedly — monitor may have crashed.\n");
    fflush(stdout);
    fclose(pipe_in);
    waitpid(mon_pid, NULL, 0);
    exit(0);
}

static void start_monitor(void){
    pid_t hub_mon_pid = fork();
    if (hub_mon_pid < 0) { perror("start_monitor: fork"); return; }

    if (hub_mon_pid == 0) {
        hub_mon();   
        exit(0);
    }

    printf("[hub] hub_mon started (PID %ld). Monitor output will appear here.\n",
           (long)hub_mon_pid);
    fflush(stdout);
}

static ssize_t read_line_from_fd(int fd, char *buf, size_t maxlen){
    size_t i = 0;
    char c;
    while (i < maxlen - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n < 0) return -1;
        if (n == 0) break;
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

static pid_t scorer(const char *district, int *read_fd_out){
    int pipefd[2];
    if (pipe(pipefd) < 0) { perror("calculate_scores: pipe"); return -1; }

    pid_t pid = fork();
    if (pid < 0) {
        perror("calculate_scores: fork");
        close(pipefd[0]); close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("scorer: dup2"); exit(1);
        }
        close(pipefd[1]);
        execl(SCORER_EXE, SCORER_EXE, district, (char *)NULL);
        perror("scorer: execl district_scorer");
        exit(1);
    }

    close(pipefd[1]);
    *read_fd_out = pipefd[0];
    return pid;
}

static void calculate_scores(char **districts, int n){
    if (n == 0) { printf("[hub] calculate_scores: no districts specified.\n"); return; }
    if (n > MAX_DISTRICTS) { printf("[hub] Too many districts (max %d).\n", MAX_DISTRICTS); return; }

    int   read_fds[MAX_DISTRICTS];
    pid_t pids[MAX_DISTRICTS];
    int   launched = 0;

    for (int i = 0; i < n; i++) {
        struct stat st;
        if (stat(districts[i], &st) < 0 || !S_ISDIR(st.st_mode)) {
            printf("[hub] District '%s' does not exist — skipping.\n",
                   districts[i]);
            read_fds[i] = -1;
            pids[i]     = -1;
            continue;
        }
        int rfd;
        pid_t pid = scorer(districts[i], &rfd);
        if (pid < 0) {
            printf("[hub] Failed to spawn scorer for '%s'\n", districts[i]);
            read_fds[i] = -1;
            pids[i]     = -1;
        } else {
            read_fds[i] = rfd;
            pids[i]     = pid;
            launched++;
        }
    }

    if (launched == 0) { printf("[hub] No scorers could be started.\n"); return; }

    printf("Workload Report\n");
    printf("%-20s %-25s %8s %8s\n", "District", "Inspector", "Score", "Reports");
    printf("%-20s %-25s %8s %8s\n", "--------", "---------", "-----", "-------");

    for (int i = 0; i < n; i++) {
        if (read_fds[i] < 0) continue;

        char line[LINE_MAX_LEN];
        while (read_line_from_fd(read_fds[i], line, sizeof(line)) > 0) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';
            if (len == 0) continue;

            char type[32];
            strncpy(type, line, sizeof(type) - 1);
            type[sizeof(type) - 1] = '\0';
            char *p = strchr(type, '|');
            if (p) *p = '\0';

            if (strcmp(type, "DISTRICT") == 0) {
                char dist[64] = "", insp[64] = "";
                int  score = 0, reports = 0;
                char tmp[LINE_MAX_LEN];
                strncpy(tmp, line, sizeof(tmp) - 1);

                char *tok = strtok(tmp,  "|"); 
                if (!tok) goto skip;
                tok = strtok(NULL, "|");       
                if (!tok) goto skip;
                strncpy(dist, tok, sizeof(dist) - 1);
                tok = strtok(NULL, "|");        
                if (!tok) goto skip;
                tok = strtok(NULL, "|");        
                if (!tok) goto skip;
                strncpy(insp, tok, sizeof(insp) - 1);
                tok = strtok(NULL, "|");        
                if (!tok) goto skip;
                tok = strtok(NULL, "|");        
                if (!tok) goto skip;
                score = atoi(tok);
                tok = strtok(NULL, "|");        
                if (!tok) goto skip;
                tok = strtok(NULL, "|");       
                if (!tok) goto skip;
                reports = atoi(tok);

                printf("%-20s %-25s %8d %8d\n", dist, insp, score, reports);

            } else if (strcmp(type, "SCORER_ERROR") == 0) {
                char *rest   = strchr(line, '|');  if (rest)   rest++;
                char *reason = rest ? strchr(rest, '|') : NULL; if (reason) reason++;
                printf("[scorer error] %s: %s\n",
                       districts[i], reason ? reason : "unknown error");

            } else if (strcmp(type, "DONE") == 0) {
                break;
            }
            skip:;
        }

        close(read_fds[i]);
        waitpid(pids[i], NULL, 0);
    }

    printf("End of Workload Report\n");
    fflush(stdout);
}

int main(void){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sa.sa_flags   = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, NULL);

    printf("City Hub\n");

    char input[1024];

    while (1) {
        printf("city_hub> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n[hub] EOF received. Exiting.\n");
            break;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') input[--len] = '\0';
        if (len == 0) continue;

        char *tokens[128];
        int   ntok = 0;
        char  copy[1024];
        strncpy(copy, input, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';

        char *tok = strtok(copy, " \t");
        while (tok && ntok < 127) { tokens[ntok++] = tok; tok = strtok(NULL, " \t"); }
        if (ntok == 0) continue;

        if (strcmp(tokens[0], "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(tokens[0], "calculate_scores") == 0) {
            calculate_scores(&tokens[1], ntok - 1);
        } else if (strcmp(tokens[0], "quit") == 0 ||
                   strcmp(tokens[0], "exit") == 0) {
            printf("[hub] Exiting.\n");
            break;
        } else {
            printf("[hub] Unknown command: '%s'", tokens[0]);
        }
    }
    return 0;
}
