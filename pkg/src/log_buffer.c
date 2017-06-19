//
// log.c -- logging code
//
// Clif Liu
//
// Copyright (c) 2015 Afero, Inc. All rights reserved.
//

#include "af_log.h"
#include <string.h>

#define BYTES_PER_ROW 32

void af_log_buffer(uint32_t level, char *name, uint8_t *buffer, int bufLen)
{
    char outBuf[BYTES_PER_ROW * 3];
    int pos = 0, i = 0, row = 0;

    if (name == NULL || buffer == NULL || bufLen < 0) {
        return;
    }

    if (g_debugLevel >= level) {
        while (bufLen--) {
            pos += sprintf(outBuf + pos, (i == 0 ? "%02x" : " %02x"), *buffer++);
            i++;
            if (i >= BYTES_PER_ROW) {
                syslog(LOG_DEBUG, "%s:%03x:%s", name, row, outBuf);
                pos = 0;
                i = 0;
                row += BYTES_PER_ROW;
            }
        }
        if (i) {
            syslog(LOG_DEBUG, "%s:%03x:%s", name, row, outBuf);
        }
    }
}

/* message looks like "<name>[<actual_len>]=<as many bytes fit>
   if the data has more than 64 bytes the message looks like
      "<name>[<actual_len>]=(truncated)<as many bytes fit>"
 */

#define TRUNCATED_MSG "(truncated)"
#define CANT_FIT_MSG  "cant_fit"
#define DATA_NULL_MSG "data_null"
#define NAME_NULL_MSG "name_null"
#define BAD_LEN_MSG   "data_len"
#define COPY_ERROR_AND_RETURN(_msg) \
    strncpy(buf, _msg, bufLen); \
    buf[bufLen - 1] = '\0'; \
    return

void af_util_convert_data_to_hex_with_name(char *name, uint8_t *data, int dataLen, char *buf, int bufLen)
{
    static char hexNybble[16] = "0123456789abcdef";

    /* check if we can't do anything */
    if (buf == NULL || bufLen < 1) {
        return;
    }

    /* check if we have other parameter errors */
    if (data == NULL) {
        COPY_ERROR_AND_RETURN(DATA_NULL_MSG);
    }

    if (name == NULL) {
        COPY_ERROR_AND_RETURN(NAME_NULL_MSG);
    }

    if (dataLen < 0) {
        COPY_ERROR_AND_RETURN(BAD_LEN_MSG);
    }

    char lengthBuf[16];
    int messageLen = strlen(name) + 3 + sprintf(lengthBuf, "%d", dataLen);
    int truncated = 0, pos;
    int outputLen = dataLen;
    if (bufLen < messageLen + dataLen * 2 + 1) {
        /* full message won't fit */
        if (bufLen < messageLen + 2 + sizeof(TRUNCATED_MSG)) {
            /* can't fit at least one hex byte and truncated message */
            COPY_ERROR_AND_RETURN(CANT_FIT_MSG);
        }
        /* truncate the data length to fit message */
        outputLen = (bufLen - messageLen - sizeof(TRUNCATED_MSG)) >> 1;
        truncated = 1;
    }

    if (truncated) {
        pos = sprintf(buf, "%s[%d]=" TRUNCATED_MSG, name, dataLen);
    } else {
        pos = sprintf(buf, "%s[%d]=", name, dataLen);
    }

    int i;
    for (i = 0; i < outputLen; i++) {
        buf[pos++] = hexNybble[data[i] >> 4];
        buf[pos++] = hexNybble[data[i] & 0x0f];
    }
    buf[pos] = '\0';

    return;
}

