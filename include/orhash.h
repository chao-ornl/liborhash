/*
 * Copyright (c) 2016       UT-Battelle, LLC
 *                          All rights reserved.
 */

#ifndef INCLUDE_ORHASH_H
#define INCLUDE_ORHASH_H

#include <mhash.h>
#include <stdio.h>
#include <stdlib.h>

#include "orhash_types.h"
#include "orhash_constants.h"

int
orhash_init (void* buffer,
             size_t buffer_size,
             size_t block_size,
             orhash_t **hash);

int
orhash_fini (orhash_t **hash);

int
orhash_set_ref_hash (orhash_t *hash);

int
orhash_compute_hash (orhash_t *orhash);

int
orhash_get_dirty_ratio (orhash_t *hash, double *ratio);

#endif
