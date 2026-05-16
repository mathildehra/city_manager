#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MONITOR_PID_FILE ".monitor_pid"

void create_hfile(){
    pid_t pid= getpid();
    char path[256];
    int fd= open(MONITOR_PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd<0) exit(1);
    int len= snprintf(path, sizeof(path), "%s", (long)pid);
    write(fd, path, (size_t)len);
    close(fd);
    printf("PID written to %s", MONITOR_PID_FILE);
}

//finish delete file
void delete_file(){
    if(unlink(MONITOR_PID_FILE)<0){
        perror("unlink .monitor_pid);
    }
}

//receives sigusr1 when a new report has been added
void sigusr1_handler(int sig){
    ()sig;
    got_sigusr1=1;
}

//process ends when receiving sigint
void sigint_handler(int sig){
    ()sig;
    got_sigint=1;
}

//add SIGCHLD handler for waitpid
void sigchld_handler(int sig){
    ()sig;
    got_sigchld=1;
}

//add function for the pipe
static void emit(const char *type, const char *text){
    time_t now = time(NULL);
    char ts[32];
    struct tm *t = localtime(&now);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

    printf("%s|[%s] %s\n", type, ts, text);
    fflush(stdout);
}

int main(){
    create_hfile();
    struct sigaction sa_usr1;
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler=sigusr1_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags=SA_RESTART;
    if(sigaction(SIGUSR1, &sa_usr1, NULL)<0){
        perror("sigaction SIGUSR1");
        exit(1);
    }
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler= sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags=0;
    if(sigaction(SIGINT, &sa_int, NULL)<0){
        perror("sigaction SIGINT");
        exit(1);
    }
    while(!got_sigint){
        pause();
        if(got_sigusr1){
            got_sigusr1=0;
            printf("SIGUSR1 received, a new report hzs been added");
        }
    }
printf("SIGINT received, monitor shutting down");
    delete_file():
    return 0;
}
