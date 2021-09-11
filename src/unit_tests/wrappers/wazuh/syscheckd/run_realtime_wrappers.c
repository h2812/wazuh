/* Copyright (C) 2021, INO Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#include "run_realtime_wrappers.h"
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int __wrap_realtime_adddir(const char *dir, int whodata, __attribute__((unused)) int followsl) {
    check_expected(dir);
    check_expected(whodata);

    return mock();
}


int __wrap_realtime_start() {
    return 0;
}

void expect_realtime_adddir_call(const char *path, int whodata, int ret) {
    expect_string(__wrap_realtime_adddir, dir, path);
    expect_value(__wrap_realtime_adddir, whodata, whodata);
    will_return(__wrap_realtime_adddir, ret);
}
