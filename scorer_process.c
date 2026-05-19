#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>


//in scorer_process.c
int scorer_process(){
    //for each inspector in that disctrict: workload score(sum of severity levels on all reports by that insoector)
    for(...){
        int workload_score=0;
        Report r;
        for(...){
            if(...){
                workload_score+=severity;
            }
        }
        return workload_score;
    }
    int pipe_scorer[2];
    int dup2(int oldfd, int mewfd);    
    fd= open("scorer.txt", O_WRONLY);
    if((newfd=dup2(fd,1))<0){
        perror("Error when call to scorer_process");
        exit(1);
    }
    printf("For inspector %s, the sum of severity levels is: %d", inspector, workload_score);
    return 0;
}
