#include "mt_modapi.h"
#include "mt_entrylist.h"
#include "mt_equation.h"
#include "mt_tags.h"
#include <cjson/cJSON.h>
#include <glib.h>

void mt_entry_begin(MT_EntryList *const entry_list, const char *const id)
{
    mt_tag_set_clear(entry_list->entry.tags);
    entry_list->entry.id = mt_tag_storage_push(entry_list->tag_storage, id);
}

void mt_entry_push_tag(MT_EntryList *const entry_list, const char *const tag)
{
    const char *const persistent_id = mt_tag_storage_push(entry_list->tag_storage, tag);
    mt_tag_set_push(entry_list->entry.tags, persistent_id);
}

bool mt_entry_is_suitable(MT_EntryList *const entry_list)
{
    mt_tag_set_push(entry_list->entry.tags, entry_list->entry.id);
    mt_tag_set_push(entry_list->entry.tags, entry_list->entry.module);
    const MT_EquationBool result = mt_equation_evaluate(entry_list->topic.equation, entry_list->entry.tags);
    return (result == MT_EquationBool_True);
}

void mt_entry_set_value_number(MT_EntryList *const entry_list, const double value)
{
    cJSON_AddItemToObject(entry_list->topic.json, entry_list->entry.id, cJSON_CreateNumber(value));
}

void mt_entry_set_value_bool(MT_EntryList *const entry_list, const bool value)
{
    cJSON_AddItemToObject(entry_list->topic.json, entry_list->entry.id, cJSON_CreateBool(value));
}

void mt_entry_set_value_null(MT_EntryList *const entry_list)
{
    cJSON_AddItemToObject(entry_list->topic.json, entry_list->entry.id, cJSON_CreateNull());
}

void mt_entry_set_value_string(MT_EntryList *const entry_list, const char *const string)
{
    cJSON_AddItemToObject(entry_list->topic.json, entry_list->entry.id, cJSON_CreateString(string));
}