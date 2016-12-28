/*
 * Copyright (c) 2016      UT-Battelle, LLC
 *                         All rights reserved.
 *
 */

#include "orhash.h"

static void
_print_orhash_metadata (orhash_t *orhash)
{
    if (orhash == NULL)
        return;

    printf ("Hash @: %p\n", (void*)orhash);
    printf ("Buffer @: %p\n", orhash->buffer);
    printf ("Buffer size: %zd\n", orhash->buffer_size);
    printf ("Block size: %zd\n", orhash->block_size);
    printf ("Number of blocks: %zd\n", orhash->num_blocks);
    printf ("Last block size: %zd\n", orhash->last_block_size);
    printf ("Array of block hashes: %p\n", (void*)orhash->hash);
    printf ("Array of block reference hashes: %p\n", (void*) orhash->ref_hash);
    printf ("Block hash start index: %d\n", orhash->hash_start_index);
    printf ("Block reference hash start index: %d\n", orhash->refhash_start_index);
}

/* The index is the block number, not the index of the block */
/* All new blocks are pratically added at the end the array even if
   logically they are added in front of the array. We can therefore have
   the following logical indexes: (-1, 0, 1, 2, 3) for an array of size 5
   which is stored as follow (0, 1, 2, -1, 3) where a block was first
   added to the front and then another block to the end */
static blockhash_t *
_find_block_hash (orhash_t *orhash, int index)
{
    int logical_index;
    int i;

    if (orhash == NULL)
        return NULL;

    /* We need to calculate the logical index we are looking for */
    logical_index = orhash->hash_start_index + index;

    for (i = 0; i < orhash->num_blocks; i++)
    {
        if (orhash->hash[i]->index == logical_index)
            return orhash->hash[i];
    }

    return NULL;
}

static blockhash_t *
_find_block_refhash (orhash_t *orhash, int index)
{
    int logical_index;
    int i;

    if (orhash == NULL)
        return NULL;

    /* We need to calculate the logical index we are looking for */
    logical_index = orhash->refhash_start_index + index;

    for (i = 0; i < orhash->num_blocks; i++)
    {
        if (orhash->ref_hash[i]->index == logical_index)
            return orhash->ref_hash[i];
    }

    return NULL;
}

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
_compute_block_hash (orhash_t *orhash, blockhash_t *block_hash, int block_index, size_t size)
{
    void    *ptr;
    MHASH   td;
    int     internal_block_index;

    if (orhash == NULL || block_hash == NULL)
        return ORHASH_ERR_BAD_PARAM;

    td = mhash_init (MHASH_ADLER32);

    if (td == MHASH_FAILED)
    {
        return ORHASH_ERROR;
    }

    ptr = orhash->buffer + (block_index * orhash->block_size);

    mhash (td, ptr, size);

    mhash_deinit (td, block_hash->hash);

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

void
orhash_print (orhash_t *orhash)
{
    int         i;
    blockhash_t *blockhash;

    if (orhash == NULL)
        return;

    _print_orhash_metadata (orhash);
    
    printf ("Block hashes:\n");
    for (i = 0; i < orhash->num_blocks; i++)
    {
        blockhash = _find_block_hash (orhash, i);
        if (blockhash == NULL)
        {
            fprintf (stderr, "Block hash not found\n");
            return;
        }

        _print_hash (blockhash->hash);
    }

    printf ("Block reference hashes:\n");
    for (i = 0; i < orhash->num_blocks; i++)
    {   
        blockhash = _find_block_refhash (orhash, i);
        if (blockhash == NULL)
        {
            fprintf (stderr, "Block hash not found\n");
            return;
        }

        _print_hash (blockhash->hash);
    }
}

int
orhash_compute_hash (orhash_t *orhash)
{
    int         i;
    int         rc;
    blockhash_t *block_hash;

    if (orhash == NULL)
        return ORHASH_ERR_BAD_PARAM;

    if (orhash->num_blocks > 1)
    {
        for (i = 0; i < orhash->num_blocks - 1; i++)
        {
            block_hash = _find_block_hash (orhash, i);
            if (block_hash == NULL)
                return ORHASH_ERROR;

            rc = _compute_block_hash (orhash, block_hash, i, orhash->block_size);
            if (rc != ORHASH_SUCCESS)
            {
                return ORHASH_ERROR;
            }
        }
    }


    /* The last block is a special case because its data size is not necessarily the block size */
    block_hash = _find_block_hash (orhash, orhash->num_blocks - 1);
    if (block_hash == NULL)
        return ORHASH_ERROR;

    rc = _compute_block_hash (orhash, block_hash, orhash->num_blocks - 1, orhash->last_block_size);
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

        hash1 = orhash->hash[i]->hash;
        hash2 = orhash->ref_hash[i]->hash;

        memcpy (hash2, hash1, HASH_LEN * sizeof (unsigned char));
    }

    orhash->refhash_start_index = orhash->hash_start_index;

    return ORHASH_SUCCESS;
}

int
orhash_reinit (orhash_t *hash_in,
               void     *buffer,
               size_t   buffer_size,
               long     block_offset)
{
    long    n_new_blocks;
    size_t  old_num_blocks;
    int     i;
    int     rc;

    if (hash_in == NULL)
        return ORHASH_ERR_BAD_PARAM;

    if (buffer_size < hash_in->buffer_size)
    {
        fprintf (stderr, "The buffer shrunk, operation not supported yet\n");
        goto exit_on_error;
    }

    /* Calculate the new number of blocks */
    if (buffer_size > hash_in->buffer_size)
    {
        /* Blocks were added to the buffer, i.e., the buffer is now bigger */
        old_num_blocks = hash_in->num_blocks;

        if (block_offset < 0)
        {   
            n_new_blocks = -block_offset;
        } else {
            n_new_blocks = block_offset;
        }

        hash_in->num_blocks += n_new_blocks;


        /* Adjust the hashes for all blocks */
        hash_in->hash = realloc (hash_in->hash, hash_in->num_blocks * sizeof (blockhash_t*));
        for (i = old_num_blocks; i < hash_in->num_blocks; i++)
        {
            int relative_idx = 0;

            hash_in->hash[i] = (blockhash_t*) malloc (sizeof (blockhash_t));
            if (hash_in->hash[i] == NULL)
                return ORHASH_ERROR;

            hash_in->hash[i]->hash = calloc (HASH_LEN, sizeof (unsigned char));
            if (hash_in->hash[i]->hash == NULL)
                return ORHASH_ERROR;

            if (block_offset < 0)
            {
                hash_in->hash[i]->index = hash_in->hash_start_index - relative_idx - 1;
            } else {
                hash_in->hash[i]->index = hash_in->num_blocks + relative_idx;
            }
        }

        /* Adjust the start index; remember that because we want to handle the shift
           of blocks without ending up with wrongly high dirty ratios, the start index
           is negative when blocks have been added to the front of the list */
        if (block_offset < 0)
            hash_in->hash_start_index = hash_in->hash_start_index + block_offset;
    } else {
        /* The buffer size did not change so we need to replace some block hashes */
        if (block_offset < 0)
        {
            /* New blocks are at the front so dropping blocks that are at the end */
            for (i = 0; i < -block_offset; i++)
            {
                rc =_compute_block_hash (hash_in,
                                         hash_in->hash[hash_in->num_blocks - 1 - i],
                                         i,
                                         hash_in->block_size);
                if (rc != ORHASH_SUCCESS)
                {
                    goto exit_on_error;
                }

                hash_in->hash[hash_in->num_blocks - 1 - i]->index = block_offset + i;
            }

            hash_in->hash_start_index = hash_in->hash_start_index + block_offset;
        } else {
            /* New blocks are at the end so dropping blocks that are at the front */
            for (i = 0; i < block_offset; i++)
            {
                rc = _compute_block_hash (hash_in,
                                          hash_in->hash[i],
                                          i,
                                          hash_in->block_size);
                if (rc != ORHASH_SUCCESS)
                {
                    goto exit_on_error;
                }

                hash_in->hash[i]->index = hash_in->num_blocks + i;
            }

            hash_in->hash_start_index = hash_in->hash_start_index + block_offset;
       }
    }

    return ORHASH_SUCCESS;

 exit_on_error:
    return ORHASH_ERROR;
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

    _h->buffer              = buffer;
    _h->buffer_size         = buffer_size;
    _h->block_size          = block_size;
    _h->num_blocks          = _calculate_num_blocks (buffer_size, block_size);
    _h->last_block_size     = _calculate_last_block_size (buffer_size, block_size, _h->num_blocks);
    _h->hash_start_index    = 0;
    _h->refhash_start_index = 0;

    _h->hash = (blockhash_t**) malloc (_h->num_blocks * sizeof (blockhash_t*));
    if (_h->hash == NULL)
        return ORHASH_ERROR;

    for (i = 0; i < _h->num_blocks; i++)
    {
        _h->hash[i] = (blockhash_t*) malloc (sizeof (blockhash_t));
        if (_h->hash[i] == NULL)
        {
            return ORHASH_ERROR;
        }

        _h->hash[i]->hash = calloc (HASH_LEN, sizeof (unsigned char));
        if (_h->hash[i]->hash == NULL)
        {
            return ORHASH_ERROR;
        }
        _h->hash[i]->index = i;
    }

    _h->ref_hash = (blockhash_t**) malloc (_h->num_blocks * sizeof (blockhash_t*));
    if (_h->ref_hash == NULL)
        return ORHASH_ERROR;

    for (i = 0; i < _h->num_blocks; i++)
    {
        _h->ref_hash[i] = (blockhash_t*) malloc (sizeof (blockhash_t));
        if (_h->ref_hash[i] == NULL)
        {
            return ORHASH_ERROR;
        }

        _h->ref_hash[i]->hash = calloc (HASH_LEN, sizeof (unsigned char));
        if (_h->ref_hash[i]->hash == NULL)
        {
            return ORHASH_ERROR;
        }
        _h->ref_hash[i]->index = i;
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
            free (_h->hash[i]->hash);
            free (_h->hash[i]);
            _h->hash[i] = NULL;
        }
    }

    for (i = 0; i < _h->num_blocks; i++)
    {
        if (_h->ref_hash[i] != NULL)
        {
            free (_h->ref_hash[i]->hash);
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
        hash1 = hash->hash[i]->hash;
        hash2 = hash->ref_hash[i]->hash;

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

