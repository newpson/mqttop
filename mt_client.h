#pragma once

typedef struct MT_Client MT_Client;

MT_Client *mt_client_new(const char *const json_config_contents);
bool mt_client_run(MT_Client *const context);
void mt_client_destroy(MT_Client *const context);