/*
 * af_util.h
 *
 * This file contains implementation of different utilities
 *
 * Copyright (c) 2017-present, Afero Inc. All rights reserved.
 *
 */
#include <sys/wait.h>

extern int af_util_system(const char *format, ...);

extern int8_t af_util_file_exists(const char *filename);
extern uint32_t af_util_read_file(const char *filename, char *buf, size_t  n);
char *af_util_buffer_to_hex(char *dest, size_t dest_len, const uint8_t *source, size_t source_len);
size_t af_util_hex_to_buffer(uint8_t *dest, size_t dest_len, const char *source, size_t source_len);
