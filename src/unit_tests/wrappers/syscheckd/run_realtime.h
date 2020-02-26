/*
 * Copyright (C) 2015-2019, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#ifndef UNIT_TEST_WRAPPERS_RUN_REALTIME
#define UNIT_TEST_WRAPPERS_RUN_REALTIME

#ifdef WIN32
#include <windows.h>

HANDLE wrap_CreateEvent (LPSECURITY_ATTRIBUTES lpEventAttributes, WINBOOL bManualReset, WINBOOL bInitialState, LPCSTR lpName);

#endif
#endif
