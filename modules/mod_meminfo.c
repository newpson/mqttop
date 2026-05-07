#include "mt_modapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *mt_module_init()
{
    return NULL;
}

void mt_module_gather(MT_EntryList *const entry, void *)
{
    FILE *file_meminfo = fopen("/proc/meminfo", "r");
    char line_buffer[128];
    while (!feof(file_meminfo)) {
        fgets(line_buffer, 128, file_meminfo);
        char *line_iter = strchr(line_buffer, ':');
        if (line_iter != NULL) {
            *line_iter = '\0';
            mt_entry_begin(entry, line_buffer);
            mt_entry_push_tag(entry, "proc");
            if (mt_entry_is_suitable(entry)) {
                ++line_iter;
                const double value = strtod(line_iter, NULL);
                mt_entry_set_value_number(entry, value);
            }
        }
    }
}

void mt_module_free(void *)
{
    return;
}