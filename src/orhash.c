/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#include "orhash.h"

static void
_print_hash (unsigned char *hash)
{
    int             i;
    MHASH           td;

    printf ("Hash: ");
    for (i = 0; i < mhash_get_block_size (MHASH_ADLER32); i++)
    {
        printf ("%.2x", hash[i]);
    }
    printf ("\n");
}

static int
_compare_hash (unsigned char *hash1, unsigned char *hash2)
{
    int i;

    for (i = 0; i < mhash_get_block_size (MHASH_ADLER32); i++)
    {
        if (hash1[i] != hash2[i])
        {
            return ORHASH_HASHES_DIFFER;
        }
    }

    return ORHASH_EQUAL_HASHES;
}

static int
_compute_block_hash (orhash_t *orhash, int block_index, size_t size)
{
    void    *ptr;
    MHASH   td;

    if (orhash == NULL)
        return ORHASH_ERR_BAD_PARAM;

    td = mhash_init (MHASH_ADLER32);

    if (td == MHASH_FAILED)
    {
        return ORHASH_ERROR;
    }

    ptr = orhash->buffer + (block_index * orhash->block_size);

    mhash (td, ptr, size);

    mhash_deinit (td, orhash->hash[block_index]);

    return ORHASH_SUCCESS;
}

static size_t
_calculate_num_blocks (size_t buffer_size, size_t block_size)
{
    div_t o;

    o = div (buffer_size, block_size);

    if (o.rem != 0)
    {
        return (o.quot + 1);
    } else {
        return (o.quot);
    }
}

static size_t
_calculate_last_block_size (size_t buffer_size, size_t block_size, size_t num_blocks)
{
    size_t _s;

    _s = buffer_size - ((num_blocks - 1) * block_size);

    return _s;
}

int
orhash_compute_hash (orhash_t *orhash)
{
    int i;
    int rc;

    if (orhash == NULL)
        return ORHASH_ERR_BAD_PARAM;

    if (orhash->num_blocks > 1)
    {
        for (i = 0; i < orhash->num_blocks - 1; i++)
        {
            rc = _compute_block_hash (orhash, i, orhash->block_size);
            if (rc != ORHASH_SUCCESS)
            {
                return ORHASH_ERROR;
            }
        }
    }


    /* The last block is a special case */
    rc = _compute_block_hash (orhash, orhash->num_blocks - 1, orhash->last_block_size);
    if (rc != ORHASH_SUCCESS)
    {
        return ORHASH_ERROR;
    }

    return ORHASH_SUCCESS;
}

int
orhash_set_ref_hash (orhash_t *orhash)
{
    int i;
    int j;

    if (orhash == NULL)
        return ORHASH_ERR_BAD_PARAM;


    for (i = 0; i < orhash->num_blocks; i++)
    {
        unsigned char *hash1;
        unsigned char *hash2;

        hash1 = orhash->hash[i];
        hash2 = orhash->ref_hash[i];

        memcpy (hash2, hash1, HASH_LEN * sizeof (unsigned char));
    }

    return ORHASH_SUCCESS;
}

int
orhash_init (void       *buffer,
             size_t     buffer_size,
             size_t     block_size,
             orhash_t   **hash)
{
    int         i;
    orhash_t    *_h;

    if (*hash != NULL)
    {
        _h = *hash;
    } else {
        _h = malloc (sizeof (orhash_t));
    }

    if (_h == NULL)
    {
        return ORHASH_ERROR;
    }

    _h->buffer          = buffer;
    _h->buffer_size     = buffer_size;
    _h->block_size      = block_size;
    _h->num_blocks      = _calculate_num_blocks (buffer_size, block_size);
    _h->last_block_size = _calculate_last_block_size (buffer_size, block_size, _h->num_blocks);

    _h->hash = (unsigned char**) malloc (_h->num_blocks * sizeof (unsigned char *));
    if (_h->hash == NULL)
        return ORHASH_ERROR;

    for (i = 0; i < _h->num_blocks; i++)
    {
        _h->hash[i] = calloc (HASH_LEN, sizeof (unsigned char));
        if (_h->hash[i] == NULL)
        {
            return ORHASH_ERROR;
        }
    }

    _h->ref_hash = (unsigned char**) malloc (_h->num_blocks * sizeof (unsigned char *));
    if (_h->ref_hash == NULL)
        return ORHASH_ERROR;

    for (i = 0; i < _h->num_blocks; i++)
    {
        _h->ref_hash[i] = calloc (HASH_LEN, sizeof (unsigned char));
        if (_h->ref_hash[i] == NULL)
        {
            return ORHASH_ERROR;
        }
    }

    *hash = _h;

    return ORHASH_SUCCESS;
}

int
orhash_fini (orhash_t **hash)
{
    int         i;
    orhash_t    *_h;

    if (hash == NULL || *hash == NULL || (*hash)->hash == NULL)
        return ORHASH_SUCCESS;

    _h = *hash;

    for (i = 0; i < _h->num_blocks; i++)
    {
        if (_h->hash[i] != NULL)
        {
            free (_h->hash[i]);
            _h->hash[i] = NULL;
        }
    }

    for (i = 0; i < _h->num_blocks; i++)
    {
        if (_h->ref_hash[i] != NULL)
        {
            free (_h->ref_hash[i]);
            _h->ref_hash[i] = NULL;
        }
    }

    free (_h->hash);
    _h->hash = NULL;
    free (_h->ref_hash);
    _h->ref_hash = NULL;

    free (*hash);
    *hash = NULL;

    return ORHASH_SUCCESS;
}

int
orhash_get_dirty_ratio (orhash_t *hash, double *ratio)
{
    int             i;
    double          n_similar   = 0.0;
    double          n_differ    = 0.0;
    double          dirty_ratio = 0.0;
    unsigned char   *hash1;
    unsigned char   *hash2;

    for (i = 0; i < hash->num_blocks; i++)
    {
        hash1 = hash->hash[i];
        hash2 = hash->ref_hash[i];

        if (_compare_hash(hash1, hash2) == ORHASH_EQUAL_HASHES)
        {
            n_similar++;
        } else {
            n_differ++;
        }
    }

    dirty_ratio = n_differ / (n_similar + n_differ);
    *ratio = dirty_ratio;

    return ORHASH_SUCCESS;
}

