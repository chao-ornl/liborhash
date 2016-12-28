/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#include "orhash.h"

#define ARRAY_SIZE  (10)

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

    orhash_print (hash);

    /* We shift blocks to the right by 2 blocks */
    for (i = 0; i < 2; i++)
    {
        array[ARRAY_SIZE - 1  - i] = array[ARRAY_SIZE - 2 - i];
        array[0] = 32.0;
        array[1] = 42.0;
    }

    rc = orhash_reinit (hash, array, ARRAY_SIZE * sizeof (double), -2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_reinit() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    orhash_print (hash);

    rc = orhash_fini (&hash);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_fini() failed (line: %d)\n", __LINE__);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

 exit_on_failure:
    if (hash != NULL)
    {
        orhash_fini (&hash);
    }

    return EXIT_FAILURE;
}

