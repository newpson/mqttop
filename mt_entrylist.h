#pragma once
#include "mt_tags.h"
#include "mt_equation.h"
#include <cjson/cJSON.h>

typedef struct MT_EntryList MT_EntryList;

struct MT_EntryList {
    MT_TagStorage *tag_storage; // NOTE: must be mutexed in multithreaded mode
    struct {
        const MT_Equation *equation;
        cJSON *json;
    } topic;
    struct {
        const char *id;
        const char *module;
        MT_TagSet *tags;
    } entry;
};

MT_EntryList *mt_entry_list_new(MT_TagStorage *const tag_storage);
void mt_entry_list_destroy(MT_EntryList *const entry);