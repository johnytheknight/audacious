/*
 * index.c
 * Copyright 2009-2010 John Lindgren
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    provided with the distribution.
 *
 * This software is provided "as is" and without any warranty, express or
 * implied. In no event shall the authors be liable for any damages arising from
 * the use of this software.
 */

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "index.h"

struct index
{
    void * * data;
    int count, size;
    int (* compare) (const void * a, const void * b, void * data);
    void * compare_data;
};

struct index * index_new (void)
{
    struct index * index = g_slice_new (struct index);

    index->data = NULL;
    index->count = 0;
    index->size = 0;
    index->compare = NULL;
    index->compare_data = NULL;

    return index;
}

void index_free (struct index * index)
{
    g_free (index->data);
    g_slice_free (struct index, index);
}

int index_count (struct index * index)
{
    return index->count;
}

void index_allocate (struct index * index, int size)
{
    if (size <= index->size)
        return;

    if (! index->size)
        index->size = 64;

    while (size > index->size)
        index->size <<= 1;

    index->data = g_realloc (index->data, sizeof (void *) * index->size);
}

void index_set (struct index * index, int at, void * value)
{
    index->data[at] = value;
}

void * index_get (struct index * index, int at)
{
    return index->data[at];
}

static void make_room (struct index * index, int at, int count)
{
    index_allocate (index, index->count + count);

    if (at < index->count)
        memmove (index->data + at + count, index->data + at, sizeof (void *) *
         (index->count - at));

    index->count += count;
}

void index_insert (struct index * index, int at, void * value)
{
    make_room (index, at, 1);
    index->data[at] = value;
}

void index_append (struct index * index, void * value)
{
    index_insert (index, index->count, value);
}

void index_copy_set (struct index * source, int from, struct index * target,
 int to, int count)
{
    memcpy (target->data + to, source->data + from, sizeof (void *) * count);
}

void index_copy_insert (struct index * source, int from, struct index * target,
 int to, int count)
{
    make_room (target, to, count);
    memcpy (target->data + to, source->data + from, sizeof (void *) * count);
}

void index_copy_append (struct index * source, int from, struct index * target,
 int count)
{
    index_copy_insert (source, from, target, target->count, count);
}

void index_merge_insert (struct index * first, int at, struct index * second)
{
    index_copy_insert (second, 0, first, at, second->count);
}

void index_merge_append (struct index * first, struct index * second)
{
    index_copy_insert (second, 0, first, first->count, second->count);
}

void index_move (struct index * index, int from, int to, int count)
{
    memmove (index->data + to, index->data + from, sizeof (void *) * count);
}

void index_delete (struct index * index, int at, int count)
{
    index->count -= count;
    memmove (index->data + at, index->data + at + count, sizeof (void *) *
     (index->count - at));
}

static int index_compare (const void * a, const void * b, void * compare)
{
    return ((int (*) (const void *, const void *)) compare)
     (* (const void * *) a, * (const void * *) b);
}

void index_sort (struct index * index, int (* compare) (const void *, const
 void *))
{
    g_qsort_with_data (index->data, index->count, sizeof (void *),
     index_compare, (void *) compare);
}

static int index_compare_with_data (const void * a, const void * b, void *
 _index)
{
    struct index * index = _index;

    return index->compare (* (const void * *) a, * (const void * *) b,
     index->compare_data);
}

void index_sort_with_data (struct index * index, int (* compare)
 (const void * a, const void * b, void * data), void * data)
{
    index->compare = compare;
    index->compare_data = data;
    g_qsort_with_data (index->data, index->count, sizeof (void *),
     index_compare_with_data, index);
    index->compare = NULL;
    index->compare_data = NULL;
}
