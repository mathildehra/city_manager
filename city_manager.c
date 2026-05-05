#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h> 

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
    int fd;
    struct stat st;
    char path[256];
    if(stat(district, &st)==-1){
        mkdir(district, 0750);
        chmod(district, 0750);
    }
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    fd= open(path, O_CREAT, 0664);
    if(fd>=0) close(fd);
    chmod(path, 0664);

    snprintf(path, sizeof(path), "%s/district.cfg", district);
    fd= open(path, O_CREAT, 0640);
    if(fd>=0) close(fd);
    chmod(path, 0640);
        
    snprintf(path, sizeof(path), "%s/logged_district", district);
    fd= open(path, O_CREAT, 0644);
    if(fd>=0) close(fd);
    chmod(path, 0644);    

    char target[256]; 
    char linkname[256];
    snprintf(target, sizeof(target), "%s/reports.dat", district);
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district);
    symlink(target, linkname);
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
    scanf("%f", &r.latitude);
    printf("Longitude: ");
    scanf("%f", &r.longitude);
    printf("Category: "); 
    scanf("%s", &r.category);
    printf("Severity: ");
    scanf("%d", &r.severity);
    printf("Description: ");
    fgets("%s", &r.description);
    printf("Timestamp: %s", ctime(&r.timestamp));

    write(fd, &r, sizeof(Report));
    SIGUSR1(fd);
    close(fd);

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: add\n", time(NULL), role, user);
    if(int pid==-1){
        dprintf(fd, "No PID has been fond, the monitor has not been modified");
    }
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
void list(char *district, char *role, char *user){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    struct stat st;
    if(stat(path,&st)==-1) return;

    char permissions[10];
    permissions_string(st.st_mode, permissions);

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

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: list\n", time(NULL), role, user);
    close(fd);
}

// view (view details of a specific report)
void view(char *district, int id, char *role, char *user){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    int fd=open(path, O_RDONLY);
    if(fd<0) return;
    
    Report r;
    while(read(fd, &r, sizeof(Report))==sizeof(Report)){
        if(r.id==id){
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

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: view\n", time(NULL), role, user);
    close(fd);
}

// remove report (removes a report given its id)
void remove_report(char *district, int id, char *role, char *user){
    int fd;
    if(strcmp(role, "manager")!=0){
        printf("only managers can remove a report");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    fd=open(path, O_RDWR);
    if(fd<0) return;

    struct stat st;
    stat(path, &st);

    int total= st.st_size / sizeof(Report);
    Report *arr = malloc(st.st_size);
    read(fd, arr, st.st_size);
    int found=-1;
    for(int i=0; i<total; i++){
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

    if(stat(path, &st)==0){
        printf("Report removed successfully");
    }
    snprintf(path, sizeof(path), "%s/logged_district", district);
    fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: remove report\n", time(NULL), role, user);
    close(fd);
}
// updated threshold
void update_threshold(char *district, int value, char *role, char *user){
    if(strcmp(role, "manager")!=0){
        printf("only managers can update the treshold");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/district.cfg", district);

    struct stat st;
    if(stat(path, &st)==-1){
        printf("stat failed");
        return;
    }

    mode_t permissions = st.st_mode & 0777;
    if(permissions != 0640){
        printf("permissions changed, expected 0640, found %o", permissions);
        return;
    }
    
    int fd=open(path, O_WRONLY | O_TRUNC);
    if(fd<0) return;

    dprintf(fd, "%d\n", value);
    close(fd);
    printf("Threshold updated successufully to %d\n", value);

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: update threshold\n", time(NULL), role, user);
    close(fd);

}

//parse condition
int parse_condition(const char *input, const char *field, const char *op, const char *value){
    const char *c1, *c2;

    if(input == NULL || field == NULL || op == NULL || value == NULL) return 0;

    //first ':'
    c1 = strchr(input, ':');
    if (c1 == NULL)   return 0;

    //second ':'
    c2 = strchr(c1 + 1, ':');
    if (c2 == NULL)   return 0;

    // field
    strncpy( field, input, c1 - input);
    field[c1-input] = '\0';
    // operator
    strncpy( op, c1+1, c2 - c1 -1);
    field[c1-input] = '\0';
    // value
    strcpy(value, c2+1);

    return 1;
}

// match condition
int match_condition(Report *r, const char *field, const char *op, const char *value){
    int num;
    time_t t;

    //severity
    if(strcmp(field, "severity")==0){
        num = atoi(value);
        if (strcmp(op, "==") ==0) return r->severity == num;
        if (strcmp(op, "!=") ==0) return r->severity != num;
        if (strcmp(op, "<") ==0) return r->severity < num;
        if (strcmp(op, "<=") ==0) return r->severity <= num;
        if (strcmp(op, ">") ==0) return r->severity > num;
        if (strcmp(op, ">=") ==0) return r->severity >= num;
    }
    // category
    if (strcmp(field, "category") ==0){
        if (strcmp(op, "==") ==0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") ==0) return strcmp(r->category, value) != 0;
    }
    // inspector
    if(strcmp(field, "inspector")==0){
        if (strcmp(op, "==") ==0) return strcmp(r->inspector,value) == 0;
        if (strcmp(op, "!=") ==0) return strcmp(r->inspector, value) != 0;
    }
    // timestamp
    if(strcmp(field, "timestamp")==0){
        t = (time_t)atol(value);
        if (strcmp(op, "==") ==0) return r->timestamp == t;
        if (strcmp(op, "!=") ==0) return r->timestamp != t;
        if (strcmp(op, "<") ==0) return r->timestamp < t;
        if (strcmp(op, "<=") ==0) return r->timestamp <= t;
        if (strcmp(op, ">") ==0) return r->timestamp > t;
        if (strcmp(op, ">=") ==0) return r->timestamp >= t;
    }
    return 0;
}

void filter(char *district, char *condition, char *role, char *user){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = opend(path, O_RDONLY);
    if(fd<0) return;

    Report r;
    char field[50], op[10], value[50];
    
    if(!parse_condition(condition, field, op, value)){
        close(fd);
        return;
    }

    while(read(fd, &r, sizeof(Report))==sizeof(Report)){
        
        if(match_condition(&r, field, op, value)){
            printf("Report ID: %d\n", r.id);
            printf("Inspector: %s\n", r.inspector);
            printf("Latitude: %f\n", r.latitude);
            printf("Longitude: %f\n", r.longitude);
            printf("Category: %s\n", r.category);
            printf("Severity: %d\n", r.severity);
            printf("Description: %s\n", r.description);
            printf("Timestamp: %s", ctime(&r.timestamp));
        }
    }
    close(fd);

    snprintf(path, sizeof(path), "%s/logged_district", district);
    int fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: filter\n", time(NULL), role, user);
    close(fd);
}

//remove a directory
void remove_district(char *district, char *role, char *user){
    if(strcmp(role, "manager")!=0){
        printf("only managers can remove a report");
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/district", district);

    struct stat st;
    stat(path, &st);

    if(stat(path, &st)==-1) return;

    int pid=fork();  //if pid=-1 error, if pid==0 in child process, pid>0 in parent prcess
    if(pid==-1){
        return;
    } else if(pid==0){
        int execlp(rm, "rm", -rf, district);
    } else if(pid>0){
        return;
    }
    
    //handler for SIGCHLD if received ->do wait/waitpid

    waitpid(); 
    if(SIGCHLD)
    

    snprintf(linkname, sizeof(linkname), "active_reports-%s", district);
    unlink(linkname);


    if(stat(path, &st)==0){
        printf("district removed successfully");
    }
    snprintf(path, sizeof(path), "%s/logged_district", district);
    fd=open(path, O_WRONLY | O_APPEND);
    if(fd<0) return;
    dprintf(fd, "Time: %ld, Role: %s, User: %s, Action: remove district\n", time(NULL), role, user);
    close(fd);
}


int main(int argc, char *argv[]){
    if(argc<7){
        printf("not enough arguments");
        return 1;
    }
    
    char *role=argv[2];
    char *user=argv[4];
    char *function=argv[5];
    char *district=argv[6];
    
    if(strcmp(function, "--add")==0){
        add(district, user, role);
    } else if (strcmp(function, "--list")==0){
        list(district, user, role);
    } else if (strcmp(function, "--view")==0){
        view(district, atoi(argv[7]), role, user);
    } else if(strcmp(function, "--remove_report")==0){
        remove_report(district, atoi(argv[7]), role, user);
    } else if (strcmp(function, "--update_threshold")==0){
        update_threshold(district, atoi(argv[7]), role, user);
    } else if(strcmp(function, "--filter")==0){
        filter(district, argv[7], role, user);
    } else if(strcmp(function, "--remove_district")==0={
        remove_district(district, role, user);  
    } else {
        printf("no function was matched");
    }
    return 0;
}
