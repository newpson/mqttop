#include "mt_entrylist.h"
#include "mt_tags.h"
#include <stddef.h>
#include <glib.h>

MT_EntryList *mt_entry_list_new(MT_TagStorage *const tag_storage)
{
    MT_EntryList *entry_list = g_new0(MT_EntryList, 1);
    entry_list->tag_storage = tag_storage;
    entry_list->entry.tags = mt_tag_set_new();
    return entry_list;
}

void mt_entry_list_destroy(MT_EntryList *const entry_list)
{
    if (entry_list == NULL)
        return;
    mt_tag_set_destroy(entry_list->entry.tags);
    g_free(entry_list);
}