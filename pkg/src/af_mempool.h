//
// mempool.h -- memory pool of fixed sized buffers
//
// Copyright (c) 2018 Afero, Inc. All rights reserved.
//

#ifndef __AF_MEMPOOL_H__
#define __AF_MEMPOOL_H__

#include <stdint.h>

#define AF_MEMPOOL_FLAG_EXPAND (1 << 0)  /* memory pool expands as needed */

typedef struct af_mempool_struct af_mempool_t;

af_mempool_t *af_mempool_create(uint32_t numUnits, uint32_t unitSize, uint32_t flags);
void *af_mempool_alloc(af_mempool_t *pool);
void af_mempool_free(void *unit);
void af_mempool_destroy(af_mempool_t *pool);
void af_mempool_log_stats(af_mempool_t *pool);

#endif // __AF_MEMPOOL_H__
