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
