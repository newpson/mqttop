#include "mt_tags.h"
#include <glib.h>
#include <stddef.h>
#include <stdlib.h>

struct MT_TagStorage {
    GStringChunk *storage;
    GHashTable *htable;
};

MT_TagStorage *mt_tag_storage_new(const size_t block_size_bytes)
{
    MT_TagStorage *tag_storage = g_new(MT_TagStorage, 1);
    tag_storage->htable = g_hash_table_new(g_str_hash, g_str_equal);
    tag_storage->storage = g_string_chunk_new(block_size_bytes);
    return tag_storage;
}

void mt_tag_storage_destroy(MT_TagStorage *const tag_storage)
{
    if (tag_storage == NULL)
        return;

    g_hash_table_destroy(tag_storage->htable);
    g_string_chunk_free(tag_storage->storage);
    g_free(tag_storage);
}

const char *mt_tag_storage_push(MT_TagStorage *const tag_storage, const char *const string)
{
    char *string_persistent;
    // NOTE: #1 Glib API has in-struct reference counting system support (which that we don't use), so they decided to sacrifice the const correctness
    if (g_hash_table_lookup_extended(tag_storage->htable, string, (void **)&string_persistent, NULL))
        return string_persistent;
    string_persistent = g_string_chunk_insert(tag_storage->storage, string);
    g_hash_table_insert(tag_storage->htable, string_persistent, NULL);
    return string_persistent;
}

const char *mt_tag_storage_lookup(const MT_TagStorage *const tag_storage, const char *const string)
{
    const char *found_key;
    // NOTE: #2 same
    g_hash_table_lookup_extended(tag_storage->htable, string, (void **)&found_key, NULL);
    return found_key;
}

MT_TagSet *mt_tag_set_new()
{
    return g_hash_table_new(g_direct_hash, g_direct_equal);
}

void mt_tag_set_destroy(MT_TagSet *const tag_set)
{
    g_hash_table_destroy(tag_set);
}

void mt_tag_set_push(MT_TagSet *const tag_set, const char *const string)
{
    g_hash_table_insert(tag_set, (void *)string, NULL);
}

const char *mt_tag_set_lookup(const MT_TagSet *const tag_set, const char *const string)
{
    const char *found_key;
    // NOTE: #2 same x2
    g_hash_table_lookup_extended((MT_TagSet *const) tag_set, string, (void **)&found_key, NULL);
    return found_key;
}

void mt_tag_set_clear(MT_TagSet *const tags)
{
    g_hash_table_remove_all(tags);
}