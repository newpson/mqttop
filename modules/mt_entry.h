#pragma once
#include "mt_tags.h"
#include "mt_equation.h"
#include <cjson/cJSON.h>

typedef struct MT_Entry MT_Entry;

MT_Entry *mt_entry_new(MT_TagStorage *const tag_storage);
void mt_entry_set_module_name(MT_Entry *const entry, const char *const module_name);
void mt_entry_set_equation(MT_Entry *const entry, const MT_Equation *const equation);
void mt_entry_json_new(MT_Entry *const entry);
cJSON *mt_entry_json_take(MT_Entry *const entry);
void mt_entry_destroy(MT_Entry *const entry);