/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#include "orhash.h"

#define ARRAY_SIZE  (1024)

int
main (int argc, char **argv)
{
    int         rc;
    double      array[ARRAY_SIZE];
    orhash_t    *hash = NULL;
    int         i;
    double      ratio;

    for (i = 0; i < ARRAY_SIZE; i++)
    {
        array[i] = i * 1.0;
    }

    rc = orhash_init (array, ARRAY_SIZE * sizeof (double), sizeof (double), &hash);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_init() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_compute_hash (hash);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_set_ref_hash (hash);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_set_ref_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    for (i = 0; i < ARRAY_SIZE / 2; i++)
    {
        array[i] = i * 2.0;
    }

    rc = orhash_compute_hash (hash);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_get_dirty_ratio (hash, &ratio);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_get_dirty_ratio() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }
    printf ("*** Dirty ratio: %.3f\n", ratio);

    return EXIT_SUCCESS;

 exit_on_failure:
    if (hash != NULL)
    {
        orhash_fini (&hash);
    }

    return EXIT_FAILURE;
}

