#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MONITOR_PID_FILE ".monitor_pid"
#define MONITOR_EXE  "./monitor_reports"
#define SCORER_EXE   "./district_scorer"
#define LINE_MAX_LEN 1024
#define MAX_DISTRICTS 64

static void hub_mon(){
    //create pipe before forking monitor child
    //read from the monitor's output
  int pfd[2];
  int pid;
  if(pipe(pfd)<0){
    perror("Pipe creation error\n");
    exit(1);
  } 
  if((pid=fork())<0){
    perror("Child process creation error\n");
    exit(1);
  }
  if(pid==0){
    /* Child process code */
    close(pfd[0]);
    write(pfd[1], buff, len); //write operation into the pipe
    close(pfd[1]);
    exit(0);
  }
  close(pfd[1]);
  read(pfd[0], buff, len);
  close(pfd[0]);
  //add fork() monitor reports

  
}
//start_monitor
static void start_monitor(){
  pid_t hub_mon_pid = fork();
  if(hub_mon_pid==-1){
    perror("Child process creation error\n")
    exit(1);
  } else if(hub_mon_pid==0){
    if(MONITOR_PID_FILE){
        write(pfd[1], buff, len);
        perror("Another monitor is already running, (pid %D)", int pid);
        exit(1);
    }
    hub_mon();
    exit(0);
  } else if(hub_mon_pid>0){
    return;
  }
}
/*
static ssize_t read_lines_from_fd(int fd, char *buf, size_t maxlen){
  size_t i = 0;
    char c;
    while (i < maxlen - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n < 0) return -1;
        if (n == 0) break;   
        buf[i++] = c;
        if (c == '\n') break; //EOF
    }
    buf[i] = '\0';
    return (ssize_t)i;
}
*/

//add spawn scorer withpipiefd, fork, pid

//add cmd calculate scores
static void caluclate_scores(char *district){
    char path[256];
    snprintf(path, sizeof(path), district);
    struct stat st;
    if(stat(ath, &st)==-1){
        perror("no districts found");
        exit(1);
    } 
    //loop for each distcritc
    for(...){
        //separate scoere process
    }
}

int main(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sa.sa_flags   = SA_NOCLDWAIT;   
    sigaction(SIGCHLD, &sa, NULL);

    int pipe_scorer[2];
    int scorer_pid;
    FILE *stream
    if(pipe(pipe_scorer)<0){
        perror("Pipe creation  failed");
        exit(1);
    }
    if((scorer_pid==fork())<0){
        perror("Child process creation failed\n");
        exit(1);
    } if(scorer_pid==0){
        //child process code
        close(pipe_scorer[0]);
        dup2(pipe_scorer[1],1);
        execlp("ls", "ls", "-l", NULL);
        printf("Error callling exec\n");
    }
    close(pipe_scorer[1]);
    stream=fdopen(pipe_scorer[0],"r");
    fscanf(stream,"%s", string);
    close(pipe_scorer[0]);

  
  //add loop for EOF, strip trailing newline, rokenise, dispatch

    return 0;
}

