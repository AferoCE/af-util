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
#include <ctype.h>

#include "af_log.h"
#include "af_util.h"

#define CMD_BUF_SIZE 256

extern const char REVISION[];
extern const char BUILD_DATE[];

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
    static short s_executed = 0;

    if (!s_executed)
    {
        AFLOG_INFO("start_af_util:revision=%s,build_date=%s", REVISION, BUILD_DATE);
        s_executed++;
    }

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
        fclose (fp);
    }
    else {
        AFLOG_ERR("af_util_read_file:: Unable to open the file (%s)", ((fname==NULL) ? "--":fname));
    }

    return (nread);
}

char *af_util_buffer_to_hex(char *dest, size_t dest_len, const uint8_t *source, size_t source_len) {
    static const char HEX[] = "0123456789abcdef";

    size_t needed = source_len * 2 + 1;
    if (dest_len < needed) {
        AFLOG_ERR("af_util_buffer_to_hex: insufficient space (got %zi, need %zi)", dest_len, needed);
        dest[0] = 0;
        return dest;
    }

    int i;
    for (i = 0; i < source_len; i++) {
        dest[i * 2 + 0] = HEX[source[i] >> 4];
        dest[i * 2 + 1] = HEX[source[i] & 0x0f];
    }
    dest[i * 2] = 0;
    return dest;
}

static uint8_t UNHEX[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

size_t af_util_hex_to_buffer(uint8_t *dest, size_t dest_len, const char *source, size_t source_len) {
    size_t needed = source_len / 2;
    if (dest_len < needed) {
        AFLOG_ERR("af_util_hex_to_buffer: insufficient space (got %zi, need %zi)", dest_len, needed);
        return 0;
    }

    int i;
    for (i = 0; i < source_len / 2; i++) {
        dest[i] = (UNHEX[source[i * 2] & 0x7f] << 4) + UNHEX[source[i * 2 + 1] & 0x7f];
    }
    return needed;
}

int af_util_parse_key_value_pair_file(char *path, af_key_value_pair_t *pairs, int numPairs)
{
    /* check params */
    if (path == NULL || pairs == NULL || numPairs < 0) {
        AFLOG_ERR("parse_kvp_param:path_NULL=%d,pair_NULL=%d,numPairs=%d", path==NULL, pairs==NULL, numPairs);
        errno = EINVAL;
        return -1;
    }

    /* open the key value pair file */
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        AFLOG_ERR("parse_kvp_open:errno=%d", errno);
        return -1;
    }

    int pairNum, line = 0;

    char buf[AF_PARSE_MAX_KEY_SIZE + AF_PARSE_MAX_VALUE_SIZE + 4];
    while (fgets(buf, sizeof(buf), f) != NULL) {
        char *c = buf, *key, *value;
        line++;
        while(isblank(*c)) c++;  /* skip over any leading spaces */
        if (*c == '#') {          /* skip over comments */
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
            if (*c == '\0') break;
            continue;
        }
        if (*c == '\n') continue; /* blank line */
        if (*c == '\0') break;    /* empty file */

        key = c;

        /* make sure the key starts right */
        if (!isalpha(*c) && *c != '_') {
            AFLOG_ERR("parse_kvp_bad_key_start:line=%d:key must start with an alphabetic character or '_'", line);
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
            if (*c == '\0') break;
            continue;
        }
        c++;

        /* make sure key contains the right characters */
        while (isalnum(*c) || *c == '_') c++;
        if (*c != '=' && !isblank(*c)) {
            AFLOG_ERR("parse_kvp_bad_key_body:line=%d:key must contain only alphanumeric characters and '_'", line);
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
            if (*c == '\0') break;
            continue;
        }

        /* at this point c points to the first character past the key */
        /* find the key */
        char last = *c;
        *c = '\0';
        int i;
        for (i = 0; i < numPairs; i++) {
            if (!strncmp(key, pairs[i].key, sizeof(pairs[i].key))) {
                pairNum = i;
                break;
            }
        }
        if (i >= numPairs) {
            AFLOG_WARNING("parse_kvp_key_not_found:line=%d,key=%s:key not found; ignoring", line, key);
            *c = last;
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
            if (*c == '\0') break;
            continue;
        }
        *c = last;   /* restore the character after the key */

        /* check for equals sign */
        if (*c != '=') {
            AFLOG_ERR("parse_kvp_expected_eq:line=%d:expected \"=\" character", line);
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
            if (*c == '\0') break;
            continue;
        }
        c++;

        /* check for single quote */
        if (*c != '\'') {
            AFLOG_ERR("parse_kvp_expected_quote:line=%d:expected single quote", line);
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
            if (*c == '\0') break;
            continue;
        }
        c++;

        value = c;
        /* move to the closing single quote */
        while (*c != '\'' && *c != '\n' && *c != '\0') c++;
        if (*c != '\'') {
            AFLOG_ERR("parse_kvp_unmatched_quotes:line=%d",line);
            if (*c == '\0') break;
            continue;
        }

        /* change the single quote to a '\0' to terminate the value string */
        *c = '\0';
        strncpy(pairs[pairNum].value, value, sizeof(pairs[pairNum].value));
        pairs[pairNum].value[sizeof(pairs[pairNum].value) - 1] = '\0';
        c++;

        /* skip any trailing spaces */
        while (isblank(*c)) c++;
        if (*c != '\n' && *c != '\0') {
            AFLOG_ERR("parse_kvp_unexpected_char_end:line=%d:unexpected character after quote", line);
            while (*c != '\n' && *c != '\0') c++; /* skip until the end of the line or file */
        }
        if (*c == '\0') break;
    }

    /* print an error if one has occurred */
    if (ferror(f)) {
        AFLOG_ERR("parse_kvp_fgets:errno=%d", errno);
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}
