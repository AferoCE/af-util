/*
 * af_util.c
 *
 * This file contains implementation of different utilities.
 *
 * Copyright (c) 2017-present, Afero Inc. All rights reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/wait.h>

#include "af_log.h"

#define CMD_BUF_SIZE 256


/* af_util_system
 *
 * Helper function for sending system commands
 *
 * return
 *  -1     - on error  (please see 'system' return vlaue)
 *  or status of command
 */
int af_util_system(const char *format, ...)
{
    va_list args;
    int nwritten, rc;
    char buf[CMD_BUF_SIZE];

    va_start(args,format);
    nwritten = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (nwritten >= sizeof(buf) || nwritten < 0) {
        AFLOG_ERR("af_util_system::failed to prepare command");
        return -1;
    }
    AFLOG_DEBUG2("af_util_system::%s", buf);
    rc = system(buf);
    if (rc != 0) {
        if (rc == -1) {
            AFLOG_ERR("system:returned -1; errno=%d", errno);
        } else {
            rc = WEXITSTATUS(rc);
            AFLOG_ERR("system:command returned %d", rc);
        }
        return rc;
    }
    return 0;
}

