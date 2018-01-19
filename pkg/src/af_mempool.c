//
// mempool.c -- memory pool of fixed sized buffers
//
// Copyright (c) 2018 Afero, Inc. All rights reserved.
//

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "af_log.h"
#include "af_mempool.h"

#define ALIGN8(_x) (((_x) + 0x7) & 0xfffffff8)

static uint32_t s_poolMagic = 0xf7bdedcd;
static uint32_t s_unitMagic = 0xcefabeba;

typedef struct prv_block_struct {
    struct prv_block_struct *next;
    struct prv_unit_struct *units;
} prv_block_t;

struct af_mempool_struct {
    uint32_t magic;
    uint32_t flags;
    uint32_t unitSize;
    uint32_t numUnits;
    struct prv_unit_struct *free;
    struct prv_block_struct *blocks;
};

/* magic is 0 if the block is free. Otherwise it's set to s_unitMagic */
typedef struct prv_unit_struct {
    uint32_t magic;
    union {
        struct prv_unit_struct *next;
        struct af_mempool_struct *pool;
    } u;
} prv_unit_t;

static prv_block_t *alloc_new_block(af_mempool_t *mp)
{
    /* don't check params or magic; we trust the caller */

    /* determine the unit size and block size */
    uint32_t actualUnitSize = ALIGN8(sizeof(prv_unit_t) + mp->unitSize);
    uint32_t blockSize = sizeof(prv_block_t) + actualUnitSize * mp->numUnits;

    prv_block_t *block = (prv_block_t *)calloc(1, blockSize);
    if (block == NULL) {
        AFLOG_ERR("alloc_new_block_calloc:errno=%d", errno);
        return NULL;
    }

    /* use this pointer for arithmetic */
    uint8_t *blockUInt8 = (uint8_t *)block + sizeof(prv_block_t);

    /* link the units together */
    block->units = (prv_unit_t *)blockUInt8;
    int i;
    for (i = 0; i < mp->numUnits - 1; i++) {
        ((prv_unit_t *)blockUInt8)->u.next = (prv_unit_t *)(blockUInt8 + actualUnitSize);
        blockUInt8 += actualUnitSize;
    }
    ((prv_unit_t *)blockUInt8)->u.next = NULL;

    AFLOG_DEBUG3("alloc_new_block:mp=%p,block=%p,actualUnitSize=%d,blockSize=%d", mp, block, actualUnitSize, blockSize);
    return block;
}

/* returns -1 if pointer does not point to a mempool */
static int check_mempool(const char *function, af_mempool_t *mp)
{
    if (mp == NULL) {
        AFLOG_ERR("%s_pool_null", function);
        errno = EINVAL;
        return -1;
    }
    if (mp->magic != s_poolMagic) {
        AFLOG_ERR("%s_pool_magic", function);
        errno = EINVAL;
        return -1;
    }
    return 0;
}

af_mempool_t *af_mempool_create(uint32_t numUnits, uint32_t unitSize, uint32_t flags)
{
    /* check parameters */
    if (unitSize == 0 || numUnits == 0) {
        AFLOG_ERR("af_mempool_create_param:unitSize=%d,numUnits=%d", unitSize, numUnits);
        errno = EINVAL;
        return NULL;
    }

    /* allocate the mempool structure */
    af_mempool_t *mp = (af_mempool_t *)calloc(1, sizeof(af_mempool_t));
    if (mp == NULL) {
        AFLOG_ERR("af_mempool_create_alloc_mp:errno=%d", errno);
        return NULL;
    }

    /* populate the mempool structure */
    mp->magic = s_poolMagic;
    mp->flags = flags;
    mp->unitSize = unitSize;
    mp->numUnits = numUnits;
    mp->blocks = alloc_new_block(mp);
    if (mp->blocks == NULL) {
        free(mp);
        return NULL;
    }
    mp->free = mp->blocks->units;

    AFLOG_DEBUG3("af_mempool_create:mp=%p,unitSize=%d,numUnits=%d,flags=%d", mp, mp->unitSize, mp->numUnits, mp->flags);
    return mp;
}

void *af_mempool_alloc(af_mempool_t *mp)
{
    if (check_mempool(__func__, mp) < 0) {
        return NULL;
    }

    /* allocate new blocks if there are no free blocks */
    if (mp->free == NULL) {
        if ((mp->flags & AF_MEMPOOL_FLAG_EXPAND) == 0) {
            AFLOG_ERR("af_mempool_alloc_no_expand");
            errno = ENOSPC;
            return NULL;
        } else {
            prv_block_t *block = alloc_new_block(mp);
            if (block == NULL) {
                AFLOG_ERR("af_mempool_alloc_block_alloc");
                errno = ENOSPC;
                return NULL;
            }

            /* add to mempool's block linked list */
            block->next = mp->blocks;
            mp->blocks = block;

            /* set free list to point to new blocks */
            mp->free = block->units;
        }
    }
    /* at this point we're guaranteed that there are blocks in free list */

    /* allocate the block */
    prv_unit_t *u = mp->free;
    mp->free = u->u.next;

    /* set up the magic number */
    u->magic = s_unitMagic;
    u->u.pool = mp;

    AFLOG_DEBUG3("af_mempool_alloc:mp=%p,u=%p", mp, u);

    return (void *)(((uint8_t *)u) + sizeof(prv_unit_t));
}

void af_mempool_free(void *unit)
{
    /* check if unit is valid */
    if (unit == NULL) {
        AFLOG_ERR("af_mempool_free_unit_null");
        return;
    }
    prv_unit_t *u = (prv_unit_t *)((uint8_t *)unit - sizeof(prv_unit_t));
    if (u->magic != s_unitMagic) {
        AFLOG_ERR("af_mempool_free_unit_magic");
        return;
    }

    af_mempool_t *mp = u->u.pool;

    /* check if mempool is valid */
    if (check_mempool(__func__, mp) < 0) {
        return;
    }
    AFLOG_DEBUG3("af_mempool_free:mp=%p,u=%p", mp, u);

    /* add unit to free list */
    u->magic = 0;
    u->u.next = mp->free;
    mp->free = u;
}

void af_mempool_destroy(af_mempool_t *mp)
{
    /* check if mempool is valid */
    if (check_mempool(__func__, mp) < 0) {
        return;
    }

    prv_block_t *block = mp->blocks;
    while (block) {
        prv_block_t *next = block->next;
        free(block);
        block = next;
    }

    free(mp);
}

void af_mempool_log_stats(af_mempool_t *mp)
{
    /* check if mempool is valid */
    if (check_mempool(__func__, mp) < 0) {
        return;
    }

    /* count the number of blocks */
    int numBlocks = 0;
    prv_block_t *block = mp->blocks;
    while (block) {
        numBlocks++;
        block = block->next;
    }

    /* count the number of free units */
    int numFree = 0;
    prv_unit_t *u = mp->free;
    while (u) {
        numFree++;
        u = u->u.next;
    }

    /* get the total number of units */
    int numTotal = numBlocks * mp->numUnits;

    AFLOG_DEBUG2("af_mempool_log_stats:mp=%p,numBlocks=%d,numTotal=%d,numFree=%d,numUsed=%d,unitSize=%d",
                 mp, numBlocks, numTotal, numFree, numTotal-numFree, mp->unitSize);
}

