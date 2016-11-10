/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#ifndef INCLUDE_ORHASH_CONSTANTS_H
#define INCLUDE_ORHASH_CONSTANTS_H

#define ORHASH_ERR_BASE 0 /* For internal use only */

typedef enum orhash_rc_e {
    ORHASH_SUCCESS          = ORHASH_ERR_BASE,
    ORHASH_ERROR            = ORHASH_ERR_BASE - 1,
    ORHASH_ERR_NOT_IMPL     = ORHASH_ERR_BASE - 2,
    ORHASH_ERR_BAD_PARAM    = ORHASH_ERR_BASE - 3,
} orhash_rc_t;

typedef enum orhash_cmp_e {
    ORHASH_EQUAL_HASHES     = 0,
    ORHASH_HASHES_DIFFER,
} orhash_cmp_t;

#endif /* INCLUDE_ORHASH_CONSTANTS_H */
