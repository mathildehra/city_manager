#ifndef CITY_MANAGER_H
#define CITY_MANAGER_H

#include <sys/types.h>
#include <time.h>

#define NAME_LEN       64
#define CATEGORY_LEN   32
#define DESC_LEN       256

#define PERM_DISTRICT_DIR   0750
#define PERM_REPORTS_DAT    0664
#define PERM_DISTRICT_CFG   0640
#define PERM_LOGGED         0644

#define REPORTS_FILE    "reports.dat"
#define CFG_FILE        "district.cfg"
#define LOG_FILE        "logged_district"

#define SYMLINK_PREFIX  "active_reports-"

#define MONITOR_PID_FILE ".monitor_pid"

typedef struct {
    int     id;
    char    inspector[NAME_LEN];
    double  latitude;
    double  longitude;
    char    category[CATEGORY_LEN];    // flooding, lighting, road
    int     severity;        // 1=minor, 2=moderate, 3=critical 
    time_t  timestamp;
    char    description[DESC_LEN];
} Report;

void permissions_to_str(mode_t mode, char *buf); 
int  check_permission(const char *path, mode_t required_bit);
void log_action(const char *district, const char *role, const char *user, const char *action);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);

#endif
