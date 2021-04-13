/* Copyright (C) 2015-2021, Wazuh Inc.
 * Copyright (C) 2009 Trend Micro Inc.
 * All right reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#ifndef CLOGREADER_H
#define CLOGREADER_H

#define EVENTLOG     "eventlog"
#define EVENTCHANNEL "eventchannel"
#define OSLOG        "oslog"
#define MULTI_LINE_REGEX              "multi-line-regex"
#define MULTI_LINE_REGEX_TIMEOUT      5
#define MULTI_LINE_REGEX_MAX_TIMEOUT  120
#define DATE_MODIFIED   1
#define DEFAULT_EVENTCHANNEL_REC_TIME 5
#define DIFF_DEFAULT_SIZE 10 * 1024 * 1024
#define DIFF_MAX_SIZE (2 * 1024 * 1024 * 1024LL)

/* oslog configurations */
#define OSLOG_NAME              "USL OSLOG Super Mc Darwin" ///< Name to be displayed in the localfile' statistics

#define LOG_CMD_STR             "log"           ///< It is the name of the command used to collect macos' logs
#define LOG_STREAM_OPT_STR      "stream"        ///< "stream" is the mode in which the "log" command is running

#define STYLE_OPT_STR           "--style"       ///< This precedes the logs' output style to be used by "log stream"
#define SYSLOG_STR              "syslog"        ///< This is the style chosen to show the "log stream" output

#define PREDICATE_OPT_STR       "--predicate"   ///< This precedes the "query" filter to be used by "log stream"
#define TYPE_OPT_STR            "--type"        ///< This precedes a "type" filter to be used by "log stream"
#define LEVEL_OPT_STR           "--level"       ///< This precedes the "level" filter to be used by "log stream"

#define OSLOG_LEVEL_DEFAULT_STR "default"  ///< Represents the lowest loggin level in oslog
#define OSLOG_LEVEL_INFO_STR    "info"     ///< Represents the intermediate loggin level in oslog
#define OSLOG_LEVEL_DEBUG_STR   "debug"    ///< Represents the highest loggin level in oslog
#define OSLOG_TYPE_ACTIVITY_STR "activity" ///< Is used to filter by `activity` logs
#define OSLOG_TYPE_LOG_STR      "log"      ///< Is used to filter by `log` logs
#define OSLOG_TYPE_TRACE_STR    "trace"    ///< Is used to filter by `trace` logs
#define OSLOG_TYPE_ACTIVITY     (0x1 << 0) ///< Flag used to filter by `activity` logs
#define OSLOG_TYPE_LOG          (0x1 << 1) ///< Flag used to filter by `log` logs
#define OSLOG_TYPE_TRACE        (0x1 << 2) ///< Flag used to filter by `trace` logs

#define MAX_LOG_CMD_ARGS        14              ///< This value takes into account the largest case of use

/** This macro is used for logging the full "log stream" command with its arguments (see MAX_LOG_CMD_ARGS)**/
#define GET_LOG_STREAM_PARAMS(x)    x[0]?x[0]:"", x[1]?x[1]:"", x[2]?x[2]:"",\
                                    x[3]?x[3]:"", x[4]?x[4]:"", x[5]?x[5]:"",\
                                    x[6]?x[6]:"", x[7]?x[7]:"", x[8]?x[8]:"",\
                                    x[9]?x[9]:"", x[10]?x[10]:"",   \
                                    x[11]?x[11]:"", x[12]?x[12]:"", \
                                    x[13]?x[13]:""
#define OSLOG_START_REGEX       "^\\d\\d\\d\\d-\\d\\d-\\d\\d \\d+:\\d+:\\d+" ///< regex to determine the start of a log (unchecked)
#define OSLOG_TIMEOUT_OUT       5

#include <pthread.h>

/* For ino_t */
#include <sys/types.h>
#include "labels_op.h"
#include "expression.h"
#include "os_xml/os_xml.h"
#include "exec_op.h"

extern int maximum_files;
extern int total_files;
extern int current_files;

typedef struct _logsocket {
    char *name;
    char *location;
    int mode;
    char *prefix;
    int socket;
    time_t last_attempt;
} logsocket;

typedef struct _outformat {
    char * target;
    char * format;
} outformat;

typedef struct _logtarget {
    char * format;
    logsocket * log_socket;
} logtarget;

/* Logreader config */
/**
 * @brief Specifies end-of-line replacement type in multiline log (multi-line-regex log format)
 */
typedef enum {
    ML_REPLACE_NO_REPLACE = 0, ///< Not replace
    ML_REPLACE_NONE,           ///< Remove end of line character
    ML_REPLACE_WSPACE,         ///< Replace with a white space character (' ')
    ML_REPLACE_TAB,            ///< Replace with a tab character ('\t')
    ML_REPLACE_MAX             ///< Flow control
} w_multiline_replace_type_t;

/**
 * @brief Specifies the type of multiline matching
 */
typedef enum {
    ML_MATCH_START = 0, ///< Matches a log by its header
    ML_MATCH_ALL,       ///< Matches a log of all your content
    ML_MATCH_END,       ///< Matches a log by its tail
    ML_MATCH_MAX,       ///< Flow control
} w_multiline_match_type_t;

/**
 * @brief Context of a multiline log that was not completely written.
 * 
 * An instance of w_multiline_timeout_ctxt_t allow save the context of a log that have not yet matched with the regex.
 */
typedef struct {
    int lines_count;   ///< number of readed lines from a multiline log
    char * buffer;     ///< backup buffer. Contains readed line so far
    time_t timestamp;  ///< last successful read
} w_multiline_ctxt_t;

/**
 * @brief An instance of w_multiline_config_t represents a multiline log file and its read configuration
 */
typedef struct {
    w_expression_t * regex;                  ///< regex to identify log entries
    w_multiline_match_type_t match_type;     ///< type of multiline matching
    w_multiline_replace_type_t replace_type; ///< replacement type
    /** Max waiting time to receive a new line, once the time has expired, the collected lines are sent */
    unsigned int timeout;
    w_multiline_ctxt_t * ctxt; ///< store current status when multiline log is in process
    int64_t offset_last_read;  ///< absolut file offset of last complete multiline log processed
} w_multiline_config_t;

/**
 * @brief Context of a oslog that was not completely written.
 *
 * An instance of w_oslog_config_t allow save the context of a log that have not yet completely read.
 */
typedef struct {
    w_expression_t * start_log_regex; ///< used to check the start of a new log
    char buffer[OS_MAXSTR];           ///< store current read when os log is in process
    int newline_offset;            ///< offset of new line, used for regex matching
    time_t timestamp;  ///< last successful read
} w_oslog_ctxt_t;

/**
 * @brief An instance of w_oslog_config_t represents a OSlog (ULS) and its state
 */
typedef struct {
    w_oslog_ctxt_t ctxt;         ///< store current status when read log is in process
    char * last_read_timestamp;  ///< timestamp of last log queued (Used for only future event)
    wfd_t * log_wfd;             ///< `log stream` IPC connector
    /** Indicates if `log stream` is currently running. if not running, localfiles with oslog format will be ignored */
    bool is_oslog_running;
} w_oslog_config_t;

/* Logreader config */
typedef struct _logreader {
    off_t size;
    int ign;
    dev_t dev;

#ifdef WIN32
    HANDLE h;
    DWORD fd;
#else
    ino_t fd;
#endif

    /* ffile - format file is only used when
     * the file has format string to retrieve
     * the date,
     */
    char *ffile;
    char *file;
    char *logformat;
    w_multiline_config_t * multiline; ///< Multiline regex config & state
    w_oslog_config_t * oslog;         ///< oslog config & state
    long linecount;
    char *djb_program_name;
    char *command;
    char *alias;
    int reconnect_time;
    char future;
    long diff_max_size;
    char *query;
    int query_type;      ///< Filtering by type in oslog
    char * query_level;  ///< Filtering by level in oslog
    int filter_binary;
    int ucs2;
    outformat ** out_format;
    char **target;
    logtarget * log_target;
    int duplicated;
    char *exclude;
    wlabel_t *labels;
    pthread_mutex_t mutex;
    int exists;
    unsigned int age;
    char *age_str;

    void *(*read)(struct _logreader *lf, int *rc, int drop_it);

    FILE *fp;
    fpos_t position; // Pointer offset when closed
} logreader;

typedef struct _logreader_glob {
    char *gpath;
    char *exclude_path;
    int num_files;
    logreader *gfiles;
} logreader_glob;

typedef struct _logreader_config {
    int agent_cfg;
    int accept_remote;
    unsigned int oslog_count;
    logreader_glob *globs;
    logreader *config;
    logsocket *socket_list;
} logreader_config;

/* Frees the Logcollector config struct  */
void Free_Localfile(logreader_config * config);

/* Frees a localfile  */
void Free_Logreader(logreader * config);

/* Removes a specific localfile of an array */
int Remove_Localfile(logreader **logf, int i, int gl, int fr, logreader_glob *globf);

/**
 * @brief Get match attribute for multiline regex 
 * @param node node to find match value
 * @retval ML_MATCH_START if match is "start" or if the attribute is not present
 * @retval ML_MATCH_ALL if match is "all"
 * @retval ML_MATCH_END if match is "end"
 */
w_multiline_match_type_t w_get_attr_match(xml_node * node);

/**
 * @brief Get replace attribute for multiline regex 
 * @param node node to find match value
 * @retval ML_REPLACE_NO_REPLACE if replace is "no-replace" or if the attribute is not present
 * @retval ML_REPLACE_WSPACE if replace is "wspace"
 * @retval ML_REPLACE_TAB if replace is "tab"
 * @retval ML_REPLACE_NONE if replace is "none"
 */
w_multiline_replace_type_t w_get_attr_replace(xml_node * node);

/**
 * @brief Get timeout attribute for multiline regex
 * @param node node to find match value
 * @retval MULTI_LINE_REGEX_TIMEOUT if the attribute is invalid or not present
 * @retval timeout value otherwise
 */
unsigned int w_get_attr_timeout(xml_node * node);

/**
 * @brief Get replace type in string format
 * @param replace_type replace type of multiline matching
 * @return const char* replace type
 */
const char * multiline_attr_replace_str(w_multiline_replace_type_t replace_type);

/**
 * @brief Get match type in string format
 * @param match_type  type of multiline matching
 * @return const char* match type
 */
const char * multiline_attr_match_str(w_multiline_match_type_t match_type);


#endif /* CLOGREADER_H */
