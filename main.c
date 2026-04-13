#include <errno.h>
#include <mosquitto.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>

void send_thermal_zones(struct mosquitto *const mosq)
{
    char sys_path[128] = "/sys/class/thermal/thermal_zone";
    const size_t sys_path_i = sizeof("/sys/class/thermal/thermal_zone") - 1; // exclude '\0'
    for (size_t i = 0; i < SIZE_MAX; ++i) {
        // read type
        snprintf(&sys_path[sys_path_i], 128 - sys_path_i, "%zu/type", i);
        FILE *file = fopen(sys_path, "r");
        if (file == NULL) {
            if (errno == ENOENT)
                break;
            continue;
        }
        char type[64];
        fscanf(file, "%63s", type);
        fclose(file);

        // read temp (m°C)
        snprintf(&sys_path[sys_path_i], 128 - sys_path_i, "%zu/temp", i);
        file = fopen(sys_path, "r");
        if (file == NULL)
            continue;
        char value[64];
        fscanf(file, "%63s", value);
        fclose(file);

        mosquitto_publish(mosq, NULL, type, strlen(value), (void *)value, 0, false);
        printf("Publised: (%s: %s)\n", type, value);
    }
}

int main(void)
{
    struct sysinfo info;
    sysinfo(&info);
    printf("RAM unit (B): %u\n", info.mem_unit);
    printf("RAM total (B): %lu\n", info.totalram);
    printf("RAM hight total (B): %lu\n", info.totalhigh);
    printf("RAM free (B): %lu\n", info.freeram);
    printf("RAM used (B): %lu\n", info.totalram - info.freeram);
    printf("CPU num cores: %d\n", get_nprocs());

    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new("my_mosquitto", false, NULL);
    mosquitto_connect(mosq, "127.0.0.1", 1883, 1800);
    send_thermal_zones(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}