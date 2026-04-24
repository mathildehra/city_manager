#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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

//create a district directory
void create_district(char *district){
    struct stat st;
    char path[256];
    if(stat(district, &st)==-1){
        mkdir(district, 0750);
        chmod(district, 0750);
    }
    snprintf(path, sizeof(path), "%s/district.dat", district);
    int fd= open(path, O_CREAT, 0664);
    if(fd>=0) close(fd);
    chmod(path, 0664);

    snprintf(path, sizeof(path), "%s/district.cfg", district);
    int fd= open(path, O_CREAT, 0640);
    if(fd>=0) close(fd);
    chmod(path, 0640);

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd= open(path, O_CREAT, 0644);
    if(fd>=0) close(fd);
    chmod(path, 0644);
}

//add report to a district directory
void add(char *district, char *user, char *role){
    create_district(district);
    
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd= open(path, O_WRONLY | O_APPEND);
    if (fd<0) return;
    
    Report r;
    r.id= rand() %10000;
    r.timestamp=time(NULL);
    strncpy(r.inspector, user, MAX_STRING-1);
    r.inspector[MAX_STRING-1]= '\0';

    printf("Report ID: %d", &r.id);
    printf("Inspector: %s", &r.inspector);
    printf("Latitude: ");
    scanf("%f", &r.latitude;
    printf("Longitude: ");
    scanf("%f", &r.longitude;
    printf("Category: "); 
    scanf("%s", &r.category);
    printf("Severity: ");
    scanf("%d", &r.severity);
    printf("Description: ");
    scanf("%s", &r.description));
    printf("Timestamp: %s", &r.timestamp);

    write(fd, &r, sizeof(Report));
    close(fd);

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: add\n", time(NULL), role, user);
    close(fd);
}

//converts permissions from binary to string
void permissions_string(mode_t mode, char *out){
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}

// list (list all reports and their contents in a district)
void list(char *district){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    struct stat st;
    if(stat(path,&st)==-1) return;

    char permissions[10];
    permissions_to_string(st.st_mode, permissions);

    printf("Permissions: %s\n", permissions);
    printf("Size: %ld\n", st.st_size);
    printf("Last Modified: %s", ctime(&st.st_mtime));

    Report r;
    int fd=open(path, O_RDONLY);
    if(fd<0) return;

    while(read(fd, &r, sizeof(Report))==sizeof(Report)){
        printf("Report id: %d\n Inspector %s\n Category: %s\n Severity: %d", r.id, r.inspector, r.category, r.severity);
    }
    close(fd);
}

// view (view details of a specific report)
void view(char *district, int id){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    int fd=open(path, O_RDONLY);
    if(fd<0) return;
    
    Report r;
    while(read(fd, &r, sizeof(Report))==sizeof(Report)){
        if(r.id==id={
            printf("Report ID: %d\n", r.id);
            printf("Inspector: %s\n", r.inspector);
            printf("Latitude: %f\n", r.latitude);
            printf("Longitude: %f\n", r.longitude);
            printf("Category: %s\n", r.category);
            printf("Severity: %d\n", r.severity);            
            printf("Description: %s\n", r.description);
            printf("Timestamp: %s\n", ctime(&r.timestamp));
            close(fd);
            return;
        }
    }
printf("Report %d not found", id);
close(fd);
}

// remove report (removes a report given its id)
void remove_report(char district, int id){
    if(strcmp(role, "manager")!=0){
        printf("only managers can remove a report");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/district.dat", district);

    int fd=open(path, O_RDWR);
    if(fd<0) return;

    struct stat st;
    stat(path, &st);

    int total_size= st.st_size / sizeof(Report);
    Report *arr = malloc(st.st_size);
    read(fd, arr, st.st_size);
    int found=-1;
    for(int i=0; i<total, i++){
        if(arr[i].id==id){
            found=i;
            break;
        }
    }
    if(found==-1){
        printf("Report %d not found", id);
        free (arr);
        close(fd);
        return;
    }
    for(int i=found; i<(total-1); i++){
        arr[i]=arr[i+1];
    }
    lseek(fd, 0, SEEK_SET);
    write(fd,arr,(total-1)*sizeof(Report));
    ftruncate(fd, (total-1)*sizeof(Report));

    free(arr);
    close(fd);
}
// int filter (thing with ai)

int main(int argc, char *argv[]){
    char *role=argv[2];
    char *user=argv[4];
    char *function=argv[5];
    char *district=argv[6];
    
    if(strcmp(function, "--add")==0){
        add(district, user, role);
    } else if (strcmp(function, "--list")==0){
        list(district);
    } else if (strcmp(function, "--view")==0){
        view(district, atoi(argv[7]));
    } else if(strcmp(function, "--remove")==0){
        remove(district, role, atoi(argv[7]));
    } else if(strcmp(function, "--filter")==0){
        filter(district, argv[7]);
    }
    return 0;
}
