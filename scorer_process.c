#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define NAME_LEN      64
#define CATEGORY_LEN  32
#define DESC_LEN      256

typedef struct {
    int    id;
    char   inspector[NAME_LEN];
    double latitude;
    double longitude;
    char   category[CATEGORY_LEN];
    int    severity;
    long   timestamp;
    char   description[DESC_LEN];
} Report;

#define MAX_INSPECTORS 256

typedef struct {
    char name[NAME_LEN];
    int  score;
    int  count;
} InspectorStat;

static InspectorStat stats[MAX_INSPECTORS];
static int n_inspectors = 0;

static int find_or_insert(const char *name){
    for (int i = 0; i < n_inspectors; i++) {
        if (strncmp(stats[i].name, name, NAME_LEN) == 0)
            return i;
    }
    if (n_inspectors >= MAX_INSPECTORS) return -1;
    strncpy(stats[n_inspectors].name, name, NAME_LEN - 1);
    stats[n_inspectors].name[NAME_LEN - 1] = '\0';
    stats[n_inspectors].score = 0;
    stats[n_inspectors].count = 0;
    return n_inspectors++;
}

int main(int argc, char *argv[]){
    setlinebuf(stdout);

    if (argc < 2) {
        printf("SCORER_ERROR|unknown|Usage: scorer_process <district>\n");
        fflush(stdout);
        return 1;
    }

    const char *district = argv[1];

    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    struct stat st;
    if (stat(path, &st) < 0) {
        printf("SCORER_ERROR|%s|District or reports.dat not found\n", district);
        fflush(stdout);
        return 1;
    }
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("SCORER_ERROR|%s|Cannot open reports.dat\n", district);
        fflush(stdout);
        return 1;
    }

    Report r;
    while (read(fd, &r, sizeof(r)) == (ssize_t)sizeof(r)) {
        int idx = find_or_insert(r.inspector);
        if (idx < 0) continue;
        stats[idx].score += r.severity;
        stats[idx].count++;
    }
    close(fd);

    if (n_inspectors == 0) {
        printf("SCORER_ERROR|%s|No reports found\n", district);
        fflush(stdout);
        return 0;
    }

    for (int i = 0; i < n_inspectors; i++) {
        printf("DISTRICT|%s|INSPECTOR|%s|SCORE|%d|REPORTS|%d\n",
               district,
               stats[i].name,
               stats[i].score,
               stats[i].count);
    }

    printf("DONE|%s\n", district);
    fflush(stdout);
    return 0;
}
