/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#ifndef INCLUDE_ORHASH_TYPES_H
#define INCLUDE_ORHASH_TYPES_H

#define HASH_LEN    (32)

typedef struct blockhash_s {
    unsigned char   *hash;
    int             index;
} blockhash_t;

typedef struct orhash_s {
    void            *buffer;
    blockhash_t     **hash;
    blockhash_t     **ref_hash;
    size_t          buffer_size;
    size_t          block_size;
    size_t          num_blocks;
    size_t          last_block_size;
    int             hash_start_index;
    int             refhash_start_index;
    MHASH           td;
} orhash_t;

#endif /* INCLUDE_ORHASH_TYPES_H */
