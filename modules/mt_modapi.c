#include "mt_modapi.h"
#include "mt_entry.h"
#include "mt_equation.h"
#include "mt_tags.h"
#include <cjson/cJSON.h>
#include <glib.h>

struct MT_Entry {
    const char *id;
    const char *module_name;
    const MT_Equation *equation;
    MT_TagStorage *tag_storage; // NOTE: must be mutexed in multithreaded mode
    MT_TagSet *tags;
    cJSON *json_entries;
};

MT_Entry *mt_entry_new(MT_TagStorage *const tag_storage)
{
    MT_Entry *entry = g_new0(MT_Entry, 1);
    entry->tag_storage = tag_storage;
    entry->tags = mt_tag_set_new();
    return entry;
}

void mt_entry_set_module_name(MT_Entry *const entry, const char *const module_name)
{
    entry->module_name = module_name;
}

void mt_entry_set_equation(MT_Entry *const entry, const MT_Equation *const equation)
{
    entry->equation = equation;
}

cJSON *mt_entry_json_take(MT_Entry *const entry)
{
    cJSON *json = entry->json_entries;
    entry->json_entries = NULL;
    return json;
}

void mt_entry_json_new(MT_Entry *const entry)
{
    entry->json_entries = cJSON_CreateObject();
}

void mt_entry_destroy(MT_Entry *const entry)
{
    if (entry == NULL)
        return;
    mt_tag_set_destroy(entry->tags);
    g_free(entry);
}

void mt_entry_begin(MT_Entry *const entry, const char *const id)
{
    mt_tag_set_clear(entry->tags);
    entry->id = mt_tag_storage_push(entry->tag_storage, id);
}

void mt_entry_push_tag(MT_Entry *const entry, const char *const tag)
{
    const char *persistent_id = mt_tag_storage_push(entry->tag_storage, tag);
    mt_tag_set_push(entry->tags, persistent_id);
}

bool mt_entry_is_suitable(MT_Entry *const entry)
{
    mt_tag_set_push(entry->tags, entry->module_name);
    mt_tag_set_push(entry->tags, entry->id);
    const MT_EquationBool result = mt_equation_evaluate(entry->equation, entry->tags);
    return (result == MT_EquationBool_True);
}

void mt_entry_set_value_number(MT_Entry *const entry, const double value)
{
    cJSON_AddItemToObject(entry->json_entries, entry->id, cJSON_CreateNumber(value));
}

void mt_entry_set_value_bool(MT_Entry *const entry, const bool value)
{
    cJSON_AddItemToObject(entry->json_entries, entry->id, cJSON_CreateBool(value));
}

void mt_entry_set_value_null(MT_Entry *const entry)
{
    cJSON_AddItemToObject(entry->json_entries, entry->id, cJSON_CreateNull());
}

void mt_entry_set_value_string(MT_Entry *const entry, const char *const string)
{
    cJSON_AddItemToObject(entry->json_entries, entry->id, cJSON_CreateString(string));
}