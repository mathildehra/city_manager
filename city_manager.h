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
    char    category[CATEGORY_LEN];
    int     severity;        
    time_t  timestamp;
    char    description[DESC_LEN];
} Report;

#endif 
