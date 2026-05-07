#pragma once
#include "mt_entrylist.h"

typedef void *(*MT_Module_InitFunPtr)();
typedef void (*MT_Module_GatherFunPtr)(MT_EntryList *const entry, void *module_data);
typedef void (*MT_Module_FreeFunPtr)(void *module_data);

typedef struct {
    void *so_handle;
    void *module_data;
    const char *name; // TODO: config alias
    MT_Module_InitFunPtr init;
    MT_Module_GatherFunPtr gather;
    MT_Module_FreeFunPtr free;
} MT_Module;

MT_Module *mt_module_load(const char *const path);
void mt_module_unload(MT_Module *const module);