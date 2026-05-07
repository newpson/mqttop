#include "mt_modapi.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *mt_module_init()
{
    return NULL;
}

void mt_module_gather(MT_EntryList *const entry_list, void *)
{
    for (size_t i = 0; i < SIZE_MAX; ++i) {
        char entry_id[32];
        char path_buffer[256];
        snprintf(path_buffer, 256, "/sys/bus/cpu/devices/cpu%zu/cpufreq/scaling_cur_freq", i);
        FILE *scaling_cur_freq_file = fopen(path_buffer, "r");
        if (scaling_cur_freq_file == NULL)
            break;
        snprintf(entry_id, 32, "cpu%zu", i);
        mt_entry_begin(entry_list, entry_id);
        mt_entry_push_tag(entry_list, "sys");
        mt_entry_push_tag(entry_list, "cpu");
        mt_entry_push_tag(entry_list, "frequency");
        if (!mt_entry_is_suitable(entry_list))
            continue;
        double scaling_cur_freq = 0.0;
        fscanf(scaling_cur_freq_file, "%lf", &scaling_cur_freq);
        mt_entry_set_value_number(entry_list, scaling_cur_freq);
        fclose(scaling_cur_freq_file);
    }
}

void mt_module_free(void *)
{
    return;
}