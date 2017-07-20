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
#include "af_util.h"

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


/* af_util_file_exists
 *     Check to see if the file exists.
 *
 * return
 *  1 - file exists
 *  0 - file does NOT exist
 */
int8_t af_util_file_exists(const char *filename)
{
    if (filename != NULL) {
        if (access(filename, R_OK ) != -1 ) {
            // file exists
            return (1);
        }
    }
    return (0);
}


/* af_util_read_file
 *
 * read n size of data from the given file
 *
 * output
 * buf   - contains the data read from the file
 *
 * return
 * number of bytes read.  On error, returns zero.
 */
uint32_t af_util_read_file(const char *fname, char *buf, size_t  n)
{
    FILE  *fp = NULL;
    uint32_t  nread = 0;

    if (buf == NULL) {
        AFLOG_ERR("af_util_read_file:: invalid input buf");
        return (nread);
    }

    fp = fopen(fname, "r");
    if (fp) {
        nread = fread(buf, 1, n, fp);
    }
    else {
        AFLOG_ERR("af_util_read_file:: Unable to open the file (%s)", ((fname==NULL) ? "--":fname));
    }

    fclose (fp);
    return (nread);
}

