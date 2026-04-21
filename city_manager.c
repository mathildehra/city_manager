#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STRING 100
#define MAX_DESCRIPTION 200

typedef struct {
    int id;
    char inspector[MAX_STRING];
    float latitude;
    float longitude;
    char category[MAX_STRING]; //road/lighting/flooding
    int severity;   //1=minor, 2=moderate, 3=critical
    time_t timestamp;
    char description[MAX_DESCRIPTION];
} Report;

//add report to a district directory
void add(char *district){
    struct stat st;
    if(stat(district, &st)==-1){
        if(mkdir(district, 0750)<0)
            exit(1);
        chmod(district, 0750);
    }
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd= open(path, O_RDWR | O_CREAT | O_APPEND, 0664);
    if (fd<0){
        exit(1);
    }

    if(chmod(path, 640)<0){
        close(fd);        
        exit(1);
    }
    
    Report r;
    r.id=id+1;
    r.timestamp=time(NULL);

    printf("Report ID: ", &r.id);
    printf("Inspector: ", strncpy(r.inspector, user, MAX_STRING));
    printf("Latitude: ", scanf("%f", &r.latitude));
    printf("Longitude: ", scanf("%f", &r.longitude));
    printf("Category: ", scanf("%s", &r.category));
    printf("Severity: ", scanf("%d", &r.severity));
    printf("Description: ", scanf("%s", &r.description));
    printf("Timestamp: ", &r.timestamp);
    
    mkdir(district, 0750);
    chmod(district, 0750);


}

// list (list all reports and their contents in a district)
void list(char *district){
    DIR *current_dir = NULL;
    current_dir = opendir(".");
    struct dirent = NULL;


    //must print curent permission bit of reports.dat in symbolic form (rw-rw-r--), file size and last modification time
    //bit to bit symbol conversion
    //
    char permi;
    for(int i=0; i<=8; i++){
        if(i=="r"){
            permi.O_APPEND("r");
        } else if(i=="w"){
            
        } else if(i=="x"){

        } else if(i=="-"){

        }
    }
    if(strcmp())


}

// view (view details of a specific report)
void view(char district, int id){

}

// remove report (removes a report given its id)
void remove(char district, int id){

}
// int filter (thing with ai)

int main(int argc, char *argv[]){
    char *role=argv[2];
    char *user=argv[4];
    char *function=argv[5];
    char *district=argv[6];
    
    if(strcmp(function, "--add")==0){
        add(district, role, user);
    } else if (strcmp(function, "--list")==0){
        list(district);
    } else if (strcmp(function, "--view")==0){
        view(district, atoi(argv[7]));
    } else if(strcmp(function, "--remove")==0){
        if(role!="manager"){
            printf("Can't change the file if not manager");
            return 1;
        }
        remove(district, atoi(argv[7]));
    } else if(strcmp(function, "--filter")==0){
        filter(district, argv[7]);
    }
    return 0;
}
