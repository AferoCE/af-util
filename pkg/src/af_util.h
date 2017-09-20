/*
 * af_util.h
 *
 * This file contains implementation of different utilities
 *
 * Copyright (c) 2017-present, Afero Inc. All rights reserved.
 *
 */

#ifndef __AF_UTIL_H__
#define __AF_UTIL_H__

#include <sys/wait.h>

extern int af_util_system(const char *format, ...);

extern int8_t af_util_file_exists(const char *filename);
extern uint32_t af_util_read_file(const char *filename, char *buf, size_t  n);
char *af_util_buffer_to_hex(char *dest, size_t dest_len, const uint8_t *source, size_t source_len);
size_t af_util_hex_to_buffer(uint8_t *dest, size_t dest_len, const char *source, size_t source_len);

#define AF_PARSE_MAX_KEY_SIZE   64
#define AF_PARSE_MAX_VALUE_SIZE 64

typedef struct {
    char key[AF_PARSE_MAX_KEY_SIZE];
    char value[AF_PARSE_MAX_VALUE_SIZE];
} af_key_value_pair_t;

/* This function parses a file containing key value pairs
   The format is:

# comment
<key1>='<value1>'
<key2>='<value2>'
...

Keys can contain the characters [A-Z] | [a-z] | [0-9] | _ but cannot start with [0 - 9]
This file can be included in bash scripts.

Returns -1 if failure. errno contains the error code.
 */
extern int af_util_parse_key_value_pair_file(char *path, af_key_value_pair_t *pairs, int numPairs);

#endif // __AF_UTIL_H__
