#include <stddef.h>
#include <stdlib.h>
#include <glib.h>
#include "mt_logf.h"
#include "mt_client.h"

int main(int argc, const char *args[])
{
    const char *json_config_path = "config.json";
    if (argc >= 2)
        json_config_path = args[1];

    char *json_config;
    if (!g_file_get_contents(json_config_path, &json_config, NULL, NULL)) {
        mt_errorf("Error reading config file \"%s\"", json_config_path);
        if (argc < 2)
            mt_notef("Usage: %s [config.json]", argc == 0 ? "client" : args[0]);
        return EXIT_FAILURE;
    }

    MT_Client *client = mt_client_new(json_config);
    if (client != NULL) {
        mt_client_run(client);
        mt_client_destroy(client);
    }
    g_free(json_config);
    return 0;
}