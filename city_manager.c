#include "city_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

//translates permission bits to string
void permissions(mode_t mode, char *buf){
    buf[0]= (mode & S_IRUSR) ? 'r' : '-';
    buf[1]= (mode & S_IWUSR) ? 'w' : '-';
    buf[2]= (mode & S_IXUSR) ? 'x' : '-';
    buf[3]= (mode & S_IRGRP) ? 'r' : '-';
    buf[4]= (mode & S_IWGRP) ? 'w' : '-';
    buf[5]= (mode & S_IXGRP) ? 'x' : '-';
    buf[6]= (mode & S_IROTH) ? 'r' : '-';
    buf[7]= (mode & S_IWOTH) ? 'w' : '-';
    buf[8]= (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}

//check for each file the right permission
int check_permission(const char *path, mode_t required_bit){
    struct stat st;
    if (stat(path, &st) < 0) {
        perror(path);
        return 0;
    }
    if(st.st_mode & required_bit){
        return 1;
    } else return 0;
}

//write at the end of each action into logged_district
void operation_log(const char *district, const char *role, const char *user,   const char *action){
    char path[512];
    snprintf(path, sizeof(path),"%s/%s", district,LOG_FILE);

    struct stat st;
    if (stat( path, &st) == 0) {
        if ( !(st.st_mode & S_IWUSR)) {
            perror("write permission denied");
            return;
        }
    }
    int fd= open(path, O_WRONLY| O_APPEND| O_CREAT, PERM_LOGGED);
    if (fd < 0) { 
        perror(path); 
        return; 
    }
    fchmod(fd, PERM_LOGGED);
    time_t now = time(NULL);
    char tbuf[64];
    struct tm *tm_info = localtime(&now);
    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);
    char line[768];
    int len = snprintf(line, sizeof(line), "[%s] role: %s user: %s action: %s\n", tbuf, role, user, action);
    write(fd, line, len);
    close(fd);
}

//create a district
static void create_district(const char *district){
    struct stat st;
    if (stat(district, &st)< 0) {
        if (mkdir(district, PERM_DISTRICT_DIR) < 0) {
            perror("could not create a new district");
            exit(1);
        }
        chmod(district, PERM_DISTRICT_DIR);
    }
    char path[512];

    snprintf(path, sizeof(path) , "%s/%s", district, REPORTS_FILE);
    if (stat(path, &st) < 0) {
        int fd = open(path, O_CREAT |O_WRONLY, PERM_REPORTS_DAT);
        if (fd < 0) { 
            perror("could not find path to reports.dat"); 
            exit(1); 
        }
        close(fd);
        chmod(path, PERM_REPORTS_DAT);
    }

    snprintf(path, sizeof(path), "%s/%s", district, CFG_FILE);
    if (stat(path, &st)< 0) {
        int fd=open(path, O_CREAT | O_WRONLY, PERM_DISTRICT_CFG);
        if (fd < 0) { 
            perror("could not find path to district.cfg"); 
            exit(1); }
        write(fd, "threshold=2\n", 12);
        close(fd);
        chmod(path, PERM_DISTRICT_CFG);
    }

    snprintf(path, sizeof(path), "%s/%s", district, LOG_FILE);
    if (stat(path, &st) < 0) {
        int fd = open(path, O_CREAT | O_WRONLY, PERM_LOGGED);
        if (fd < 0) { 
            perror("could not find the path to logged_district"); 
            exit(1); 
        }
        close(fd);
        chmod(path, PERM_LOGGED);
    }

    char linkname[256];
    snprintf(linkname, sizeof(linkname), "%s%s", SYMLINK_PREFIX, district);
    char target[512];
    snprintf(target, sizeof(target), "%s/%s", district, REPORTS_FILE);

    struct stat lst;
    if (lstat(linkname, &lst) < 0) {
        if (symlink(target, linkname) < 0 && errno != EEXIST)
            perror("symlink");
    }
}

//count number of reports and give id
static int next_report_id(const char *district){
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", district, REPORTS_FILE);
    int fd = open(path, O_RDONLY);
    if (fd < 0) return 1;
    int max_id= 0;
    Report r;
    while (read(fd, &r, sizeof(r)) == (ssize_t)sizeof(r)) {
        if (r.id > max_id) max_id = r.id;
    }
    close(fd);
    return max_id+1;
}

//notify monitor function
static int notify_monitor(void){
    int fd = open(MONITOR_PID_FILE, O_RDONLY);
    if (fd < 0) return 0;

    char buf[32];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return 0;
    buf[n] = '\0';

    char *end;
    long pid = strtol(buf, &end, 10);
    if (pid <= 0) return 0;

    if (kill((pid_t)pid, SIGUSR1) < 0) return 0;
    return 1;
}

//add a report to a distric
static void add(const char *district, const char *role, const char *user){
    create_district(district);
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", district, REPORTS_FILE);

    if (!check_permission(path, S_IWGRP)) {
        fprintf(stderr, "write permission denied");
        exit(1);
    }

    Report r;
    memset(&r, 0, sizeof(r));
    r.id = next_report_id(district);
    strncpy(r.inspector, user, NAME_LEN - 1);
    printf("Latitude  : "); 
    fflush(stdout);
    if (scanf("%lf", &r.latitude) != 1) { 
        fprintf(stderr,"Invalid input\n"); 
        exit(1); 
    }
    printf("Longitude : "); 
    fflush(stdout);
    if (scanf("%lf", &r.longitude) != 1) { 
        fprintf(stderr,"Invalid input\n"); 
        exit(1); 
    }
    printf("Category (road, lighting, flooding): "); 
    fflush(stdout);
    if (scanf("%31s", r.category) != 1) { 
        fprintf(stderr,"Invalid input\n"); 
        exit(1); 
    }
    printf("Severity (1=minor, 2=moderate, 3=critical): "); 
    fflush(stdout);
    if (scanf("%d", &r.severity) != 1 || r.severity < 1 || r.severity > 3) {
        fprintf(stderr, "Invalid severity\n"); 
        exit(1);
    }

    { int c; 
     while ((c = getchar()) != '\n' && c != EOF); 
    }

    printf("Description: "); fflush(stdout);
    if (fgets(r.description, DESC_LEN, stdin) == NULL) {
        fprintf(stderr,"Invalid input\n"); exit(1);
    }
    r.description[strcspn(r.description, "\n")] = '\0';
    r.timestamp = time(NULL);

    int fd = open(path, O_WRONLY | O_APPEND);
    if (fd < 0) { 
        perror(path); 
        exit(1); }
    if (write(fd, &r, sizeof(r)) != (ssize_t)sizeof(r)) {
        perror("write"); 
        close(fd); 
        exit(1);
    }
    close(fd);
    chmod(path, PERM_REPORTS_DAT);

    printf("Report %d added to district '%s'", r.id, district);

    char action[128];
    snprintf(action, sizeof(action), "add report id %d", r.id);
    operation_log(district, role, user, action);

    char log_msg[256];
    if (notify_monitor()) {
        snprintf(log_msg, sizeof(log_msg),
                 "Monitor notified of new report id %d", r.id);
    } else {
        snprintf(log_msg, sizeof(log_msg),
                 "Monitor could not be informed of new report id %d ", r.id);
    }
    operation_log(district, role, user, log_msg);
}

//list all reports in a district
static void list(const char *district, const char *role, const char *user){
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", district, REPORTS_FILE);
    if (!check_permission(path, S_IRGRP)) {
        fprintf(stderr, "read permission denied");
        exit(1);
    }
    struct stat st;
    if (stat(path, &st) < 0) { 
        perror(path); 
        exit(1); }

    char perm_str[10];
    permissions(st.st_mode, perm_str);
    char mtime_str[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("District: %s \n", district);
    printf("File: %s  Permissions: %s  Size: %lld bytes  Modified: %s\n", REPORTS_FILE, perm_str, (long long)st.st_size, mtime_str);

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); exit(1); }

    Report r;
    while (read(fd, &r, sizeof(r)) == (ssize_t)sizeof(r)) {
        printf("Report ID: %-5d - Inspector: %-20s - Category: %-12s - Severity: %d - Latitude; %.4f - Longitude: %.4f - Time: %ld\n",
               r.id, r.inspector, r.category, r.severity, r.latitude, r.longitude, (long)r.timestamp);
    }
    close(fd);

    char linkname[256];
    snprintf(linkname, sizeof(linkname), "%s%s", SYMLINK_PREFIX, district);
    struct stat lst;
    if (lstat(linkname, &lst) == 0 && S_ISLNK(lst.st_mode)) {
        if (stat(linkname, &st) < 0)
            fprintf(stderr, "WARNING: %s is a dangling symlink!\n", linkname);
    }

    operation_log(district, role, user, "list");
}

//view the full details of a report
static void view(const char *district, int report_id, const char *role, const char *user){
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", district, REPORTS_FILE);

    if (!check_permission(path, S_IRGRP)) {
        fprintf(stderr, "read permission denied"); 
        exit(1);
    }

    int fd =open(path, O_RDONLY);
    if (fd< 0) { 
        perror(path); 
        exit(1); }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(r)) == (ssize_t)sizeof(r)) {
        if (r.id == report_id) { 
            found = 1; 
            break; 
        }
    }
    close(fd);

    if (!found) {
        fprintf(stderr, "view: report #%d not found in '%s'\n",
                report_id, district);
        exit(1);
    }

    char ts[64];
    struct tm *tm_info = localtime(&r.timestamp);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("Report %d \n", r.id);
    printf("Inspector  : %s\n",  r.inspector);
    printf("Latitude   : %.6f\n", r.latitude);
    printf("Longitude  : %.6f\n", r.longitude);
    printf("Category   : %s\n",  r.category);
    printf("Severity   : %d\n",  r.severity);
    printf("Timestamp  : %s\n",  ts);
    printf("Description: %s\n",  r.description);

    char action[64];
    snprintf(action, sizeof(action), "view report id %d", report_id);
    operation_log(district, role, user, action);
}

//remove a report
static void remove_report(const char *district, int report_id, const char *role, const char *user){
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "manager role required"); exit(1);
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", district, REPORTS_FILE);

    if (!check_permission(path, S_IWUSR)) {
        fprintf(stderr, "write permission denied"); 
        exit(1);
    }

    int fd =open(path, O_RDWR);
    if (fd <0) { 
        perror(path); 
        exit(1); }

    Report r;
    off_t target_offset = -1;
    off_t offset = 0;
    while (read(fd, &r, sizeof(r)) == (ssize_t)sizeof(r)) {
        if (r.id == report_id) target_offset = offset;
        offset += (off_t)sizeof(r);
    }

    if (target_offset < 0) {
        fprintf(stderr, "report %d not found", report_id);
        close(fd); 
        exit(1);
    }

    off_t read_pos  = target_offset + (off_t)sizeof(r);
    off_t write_pos = target_offset;

    while (1) {
        lseek(fd, read_pos, SEEK_SET);
        ssize_t n = read(fd, &r, sizeof(r));
        if (n <= 0) break;
        lseek(fd, write_pos, SEEK_SET);
        write(fd, &r, (size_t)n);
        read_pos  += n;
        write_pos += n;
    }

    ftruncate(fd, write_pos);
    close(fd);

    struct stat st;
    stat(path, &st);
    printf("removed %d from '%s'. File now %lld bytes.", report_id, district, (long long)st.st_size);

    char action[64];
    snprintf(action, sizeof(action), "remove_report id %d", report_id);
    operation_log(district, role, user, action);
}

//update threshold
static void update_threshold(const char *district, int value, const char *role, const char *user){
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "manager role required"); 
        exit(1);
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", district, CFG_FILE);

    struct stat st;
    if (stat(path, &st) < 0) { 
        perror(path); 
        exit(1); 
    }

    mode_t bits = st.st_mode & 0777;
    if (bits != PERM_DISTRICT_CFG) {
        char buf[10];
        permissions(st.st_mode, buf);
        fprintf(stderr,
                "permission mismatch on %s "
                "(found %s, expected rw-r-----). Refusing.", path, buf);
        exit(1);
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd < 0) { 
        perror(path); 
        exit(1); 
    }

    char line[64];
    int len = snprintf(line, sizeof(line), "threshold=%d", value);
    write(fd, line, len);
    close(fd);

    printf("set to %d in '%s'\n", value, district);

    char action[64];
    snprintf(action, sizeof(action), "update_threshold value=%d", value);
    operation_log(district, role, user, action);
}

//filter method
static void filter(char *district, char *condition, char *role, char *user){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int fd = open(path, O_RDONLY);
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
    operation_log(district, role, user, "filter");
}

int parse_condition(const char *input, char *field, char *op, char *value){
    const char *c1, *c2;
    if(input == NULL || field == NULL || op == NULL || value == NULL) return 0;

    c1 = strchr(input, ':');
    if (c1 == NULL)   return 0;
    c2 = strchr(c1 + 1, ':');
    if (c2 == NULL)   return 0;

    strncpy( field, input, c1 - input);
    field[c1-input] = '\0';
    strncpy( op, c1+1, c2 - c1 -1);
    op[c2-c1-1] = '\0';
    strcpy(value, c2+1);

    return 1;
}

int match_condition(Report *r, const char *field, const char *op, const char *value){
    int num;
    time_t t;

    if(strcmp(field, "severity")==0){
        num = atoi(value);
        if (strcmp(op, "==") ==0) return r->severity == num;
        if (strcmp(op, "!=") ==0) return r->severity != num;
        if (strcmp(op, "<") ==0) return r->severity < num;
        if (strcmp(op, "<=") ==0) return r->severity <= num;
        if (strcmp(op, ">") ==0) return r->severity > num;
        if (strcmp(op, ">=") ==0) return r->severity >= num;
    }
    if (strcmp(field, "category") ==0){
        if (strcmp(op, "==") ==0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") ==0) return strcmp(r->category, value) != 0;
    }
    if(strcmp(field, "inspector")==0){
        if (strcmp(op, "==") ==0) return strcmp(r->inspector,value) == 0;
        if (strcmp(op, "!=") ==0) return strcmp(r->inspector, value) != 0;
    }
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

static void remove_district(const char *district, const char *role, const char *user){
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "remove_district: manager role required"); 
        exit(1);
    }

    if (strlen(district) == 0 || strchr(district, '/') != NULL) {
        fprintf(stderr, "invalid district name '%s'", district);
        exit(1);
    }

    struct stat st;
    if (stat(district, &st) < 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "district '%s' not found", district);
        exit(1);
    }

    printf("removing district '%s'...", district);

    pid_t pid = fork();
    if (pid < 0) { 
        perror("fork"); 
        exit(1); 
    }
    if (pid == 0) {
        execlp("rm", "rm", "-rf", district, (char *)NULL);
        perror("execlp rm");
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); exit(1); }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "remove_district: rm -rf failed\n");
        exit(1);
    }

    char linkname[256];
    snprintf(linkname, sizeof(linkname), "%s%s", SYMLINK_PREFIX, district);
    if (unlink(linkname) < 0 && errno != ENOENT)
        perror("unlink symlink");

    printf("'%s' removed successfully.", district);
    (void)role; 
    (void)user;
}


int main(int argc, char *argv[]){
    if(argc<7){
        printf("not enough arguments");
        return 1;
    }
    
    char *role=argv[2];
    char *user=argv[4];
    char *command=argv[5];
    char *district=argv[6];
    
    if(strcmp(command, "--add")==0){
        add(district, user, role);
    } else if (strcmp(command, "--list")==0){
        list(district, user, role);
    } else if (strcmp(command, "--view")==0){
        view(district, atoi(argv[7]), role, user);
    } else if(strcmp(command, "--remove_report")==0){
        remove_report(district, atoi(argv[7]), role, user);
    } else if (strcmp(command, "--update_threshold")==0){
        update_threshold(district, atoi(argv[7]), role, user);
    } else if(strcmp(command, "--filter")==0){
        filter(district, argv[7], role, user);
    } else if(strcmp(command, "--remove_district")==0){
        remove_district(district, role, user);  
    } else {
        printf("no command was matched");
    }
    return 0;
}
