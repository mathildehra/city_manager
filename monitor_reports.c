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

void create_hfile(){
    struct stat st;
    char path[256];
    if(stat(monitor, &st)==-1){
    snprintf(path, sizeof(path), "%s/.monitor_pid");
    int fd= open(path, O_CREAT, 0664);
    if(fd>=0) close(fd);
    chmod(path, 0664);
    } else return;
}

//finish delete file
void delete_file(){
}

//receives sigusr1 when a new report has been added
void sigusr1_handler(int sig){
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigusr1_handler;
    sigaction(SIGUSR1, &sa, NULL);
    printf("A new file has been added");
}

//process ends when receiving sigint
void sigint_handler(int sig){
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flages = 0;
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    printf("SIGINT received, ending the process...");
}

//add SIGCHLD handler for waitpid
void sigchld_handler(int sig){
    
}

int main(){
    create_hfile();
    sigusr1_handler(sig);
    sigint_handler(sig);
    //delete at end of child process, after receiving sigint
    delete_file():
    return 0;
}
