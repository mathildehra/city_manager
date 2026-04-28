#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(){
    create_hfile();
    if(SIGUSR1()){
        printf("a new file has been created");
    }
    if(SIGINT){
        printf("SIGINT received");
    }
    remove(monitor);
    return 0;
}
