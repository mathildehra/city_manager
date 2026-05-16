#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MONITOR_EXE  "./monitor_reports"
#define SCORER_EXE   "./district_scorer"
#define LINE_MAX_LEN 1024
#define MAX_DISTRICTS 64

static void run_hub_monitor(){
  int pipefd[2];
  if(pipe(pipefd)<0){
    exit(1);
  }
  //add fork monitor reports

  //read the lines  
}
//cmd_start_monitor
static void cmd_start_monitor(){
  pid_t hub_mon_pid = fork();
  if(hub_mon_pid<0){
    return;
  }
  if(hub_mon_pid==0){
    run_hub_monitor();
    exit(0);
  }
}

static ssize_t read_lines_from_fd(int fd, char *buf, size_t maxlen){
  size_t i = 0;
    char c;
    while (i < maxlen - 1) {
        ssize_t n = read(fd, &c, 1);
        if (n < 0) return -1;
        if (n == 0) break;   /* EOF */
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    return (ssize_t)i;
}

//add spawn scorer withpipiefd, fork, pid

//add cmd calculate scores

int main(){
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags   = SA_NOCLDWAIT;   
  sigaction(SIGCHLD, &sa, NULL);

  //add loop for EOF, strip trailing newline, rokenise, dispatch

  return 0;
}
