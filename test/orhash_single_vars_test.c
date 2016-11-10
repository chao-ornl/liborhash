/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#include "orhash.h"

int
main (int argc, char **argv)
{
    int         rc;
    char        _c      = 'a';
    double      _d      = 0.0;
    orhash_t    *hash1  = NULL;
    orhash_t    *hash2  = NULL;
    double      ratio1;
    double      ratio2;

    rc = orhash_init (&_c, sizeof (char), 1, &hash1);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_init() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_init (&_d, sizeof (double), 1, &hash2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_init() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_compute_hash (hash1);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_set_ref_hash (hash1);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_set_ref_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    _c = 'b';
    rc = orhash_compute_hash (hash1);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_get_dirty_ratio (hash1, &ratio1);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_get_dirty_ratio() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }
    printf ("*** Dirty ratio: %.3f\n", ratio1);
    if (ratio1 != 1.0)
    {
        fprintf (stderr, "ERROR: the dirty ratio should be equal to 1\n");
    }

    rc = orhash_compute_hash (hash2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_set_ref_hash (hash2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_set_ref_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_get_dirty_ratio (hash2, &ratio2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_get_dirty_ratio() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }
    printf ("*** Dirty ratio: %.3f\n", ratio2);
    if (ratio2 != 0.0)
    {
        fprintf (stderr, "ERROR: the dirty ratio should be equal to 0\n");
    }

    _d = 1.0;
    rc = orhash_compute_hash (hash2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }

    rc = orhash_get_dirty_ratio (hash2, &ratio2);
    if (rc != ORHASH_SUCCESS)
    {
        fprintf (stderr, "ERROR: orhash_get_dirty_ratio() failed (line: %d)\n", __LINE__);
        goto exit_on_failure;
    }
    printf ("*** Dirty ratio: %.3f\n", ratio2);
    if (ratio2 != 0.25)
    {
        fprintf (stderr, "ERROR: the dirty ratio should be equal to 0.25\n");
    }

    if (hash1 != NULL)
    {
        orhash_fini (&hash1);
    }   

    if (hash2 != NULL)
    {
        orhash_fini (&hash2);
    }

    return EXIT_SUCCESS;

 exit_on_failure:
    if (hash1 != NULL)
    {
        orhash_fini (&hash1);
    }

    if (hash2 != NULL)
    {
        orhash_fini (&hash2);
    }

    return EXIT_FAILURE;
}
