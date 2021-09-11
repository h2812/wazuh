/*
 * Copyright (C) 2021, INO Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#ifdef __linux__
#ifdef ENABLE_AUDIT
#include "syscheck_audit.h"

volatile int audit_health_check_creation;
volatile int hc_thread_active;
pthread_mutex_t audit_hc_mutex;
pthread_cond_t audit_hc_started;


// Audit healthcheck before starting the main thread
int audit_health_check(int audit_socket) {
    int retval;
    FILE *fp;
    audit_health_check_creation = 0;
    unsigned int timer = 10;
    char abs_path_healthcheck[PATH_MAX] = {'\0'};
    char abs_path_healthcheck_file[PATH_MAX] = {'\0'};

    w_mutex_init(&audit_hc_mutex, NULL);

    // Audit needs an absolute path to add rules
    abspath(AUDIT_HEALTHCHECK_DIR, abs_path_healthcheck, PATH_MAX);
    abspath(AUDIT_HEALTHCHECK_FILE, abs_path_healthcheck_file, PATH_MAX);

    retval = audit_add_rule(abs_path_healthcheck, WHODATA_PERMS, AUDIT_HEALTHCHECK_KEY);
    if (retval <= 0 && retval != -EEXIST) {
        mdebug1(FIM_AUDIT_HEALTHCHECK_RULE);
        return -1;
    }

    mdebug1(FIM_AUDIT_HEALTHCHECK_START);

    w_cond_init(&audit_hc_started, NULL);

    // Start reading thread
    w_create_thread(audit_healthcheck_thread, &audit_socket);

    w_mutex_lock(&audit_hc_mutex);
    while (!hc_thread_active)
        w_cond_wait(&audit_hc_started, &audit_hc_mutex);
    w_mutex_unlock(&audit_hc_mutex);

    // Generate open events until they get picked up
    do {
        fp = fopen(abs_path_healthcheck_file, "w");

        if (!fp) {
            mdebug1(FIM_AUDIT_HEALTHCHECK_FILE);
        } else {
            fclose(fp);
        }

        sleep(1);
    } while (!audit_health_check_creation && --timer > 0);

    if (!audit_health_check_creation) {
        mdebug1(FIM_HEALTHCHECK_CREATE_ERROR);
        retval = -1;
    } else {
        mdebug1(FIM_HEALTHCHECK_SUCCESS);
        retval = 0;
    }

    // Delete that file
    unlink(abs_path_healthcheck_file);

    if (audit_delete_rule(abs_path_healthcheck, WHODATA_PERMS, AUDIT_HEALTHCHECK_KEY) <= 0) {
        mdebug1(FIM_HEALTHCHECK_CHECK_RULE); // LCOV_EXCL_LINE
    }
    hc_thread_active = 0;

    return retval;
}

// LCOV_EXCL_START
void *audit_healthcheck_thread(int *audit_sock) {

    w_mutex_lock(&audit_hc_mutex);
    hc_thread_active = 1;
    w_cond_signal(&audit_hc_started);
    w_mutex_unlock(&audit_hc_mutex);

    mdebug2(FIM_HEALTHCHECK_THREAD_ACTIVE);

    audit_read_events(audit_sock, HEALTHCHECK_MODE);

    mdebug2(FIM_HEALTHCHECK_THREAD_FINISHED);

    return NULL;
}
// LCOV_EXCL_STOP

#endif // ENABLE_AUDIT
#endif // __linux__
