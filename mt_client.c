#include "mt_client.h"
#include "mt_entrylist.h"
#include "mt_equation.h"
#include "mt_logf.h"
#include "mt_module.h"
#include "mt_tags.h"
#include <cjson/cJSON.h>
#include <dlfcn.h>
#include <glib.h>
#include <mosquitto.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char *client_id;
    char *address;
    int port;
    bool clean_session;
} MT_BrokerInfo;

struct MT_Client {
    // TODO: multiple brokers
    MT_TagStorage *tag_storage;
    MT_BrokerInfo broker_info;
    struct mosquitto *mosquitto;
    GPtrArray *modules;
    GPtrArray *topics;
};

typedef struct {
    char *name;
    MT_Equation *equation;
    double gather_period;
    GTimer *gather_timer;
    MT_EntryList *entry_list;
} MT_Topic;

typedef enum {
    MT_JSONFieldType_Object,
    MT_JSONFieldType_String,
    MT_JSONFieldType_Array,
    MT_JSONFieldType_Number,
    MT_JSONFieldType_Boolean,
} MT_JSONFieldType;

static const char *const MT_JSONFieldType_string[] = {
    [MT_JSONFieldType_Object] = "Object",
    [MT_JSONFieldType_String] = "String",
    [MT_JSONFieldType_Array] = "Array",
    [MT_JSONFieldType_Number] = "Number",
    [MT_JSONFieldType_Boolean] = "Boolean",
};

typedef cJSON_bool (*MT_JSONFieldType_CheckFunPtr)(const cJSON *const json_field);

static const MT_JSONFieldType_CheckFunPtr MT_JSONFieldType_function[] = {
    [MT_JSONFieldType_Object] = cJSON_IsObject,
    [MT_JSONFieldType_String] = cJSON_IsString,
    [MT_JSONFieldType_Array] = cJSON_IsArray,
    [MT_JSONFieldType_Number] = cJSON_IsNumber,
    [MT_JSONFieldType_Boolean] = cJSON_IsBool,
};

static const cJSON *mt_json_get_field_required(const cJSON *const json_parent, const char *const field, const MT_JSONFieldType type)
{
    const cJSON *const json_field = cJSON_GetObjectItemCaseSensitive(json_parent, field);
    if (json_field == NULL) {
        mt_errorf("Field not found: \"%s\"", field);
        return NULL;
    }
    if (!MT_JSONFieldType_function[type](json_field)) {
        mt_errorf("Field has wrong type (expected \"%s\"): \"%s\"", MT_JSONFieldType_string[type], field);
        return NULL;
    }
    return json_field;
}

static const cJSON *mt_json_get_array_item(const cJSON *const json_array, const size_t i, const MT_JSONFieldType type)
{
    const cJSON *const json_item = cJSON_GetArrayItem(json_array, i);
    if (!MT_JSONFieldType_function[type](json_item)) {
        mt_errorf("Array item has wrong type (expected \"%s\"): \"%zu\"", MT_JSONFieldType_string[type], i);
        return NULL;
    }
    return json_item;
}

static bool mt_broker_info_init(const cJSON *const json_parent, MT_BrokerInfo *const broker_info)
{
    const cJSON *const json_client_id = mt_json_get_field_required(json_parent, "client_id", MT_JSONFieldType_String);
    if (json_client_id == NULL)
        return false;
    broker_info->client_id = g_strdup(cJSON_GetStringValue(json_client_id));

    const cJSON *const json_address = mt_json_get_field_required(json_parent, "address", MT_JSONFieldType_String);
    if (json_address == NULL)
        return false;
    broker_info->address = g_strdup(cJSON_GetStringValue(json_address));

    const cJSON *const json_port = mt_json_get_field_required(json_parent, "port", MT_JSONFieldType_Number);
    if (json_port == NULL)
        return false;
    broker_info->port = (int)cJSON_GetNumberValue(json_port);

    const cJSON *const json_clean_session = mt_json_get_field_required(json_parent, "clean_session", MT_JSONFieldType_Boolean);
    if (json_clean_session == NULL)
        return false;
    broker_info->clean_session = cJSON_IsTrue(json_clean_session);

    return true;
}

static void mt_broker_info_free(MT_BrokerInfo *const broker_info)
{
    g_free(broker_info->client_id);
    g_free(broker_info->address);
}

static bool mt_modules_load(const cJSON *const json_parent, GPtrArray *const modules, MT_TagStorage *const tag_storage)
{
    const cJSON *const json_modules = mt_json_get_field_required(json_parent, "modules", MT_JSONFieldType_Array);
    if (json_modules == NULL)
        return false;
    const size_t num_modules = cJSON_GetArraySize(json_modules);
    size_t num_loaded_modules = 0;
    for (size_t i = 0; i < num_modules; ++i) {
        const cJSON *const json_module = mt_json_get_array_item(json_modules, i, MT_JSONFieldType_String);
        const char *const module_path = mt_tag_storage_push(tag_storage, cJSON_GetStringValue(json_module));
        MT_Module *const module = mt_module_load(module_path);
        if (module == NULL) {
            mt_warnf("Skipping module \"%s\"", module_path);
            continue;
        }
        g_ptr_array_add(modules, module);
        num_loaded_modules += 1;
    }
    if (num_loaded_modules == 0) {
        mt_warnf("No modules provided");
        mt_notef("There will be no any data in the client");
    }
    return true;
}

static void mt_topic_free(MT_Topic *const topic)
{
    if (topic == NULL)
        return;

    g_free(topic->name);
    mt_equation_free(topic->equation);
    g_timer_destroy(topic->gather_timer);
    mt_entry_list_destroy(topic->entry_list);
    g_free(topic);
}

static MT_Topic *mt_topic_new(const cJSON *const json_topic, MT_TagStorage *const tag_storage)
{
    MT_Topic *topic = g_new(MT_Topic, 1);

    const cJSON *const json_name = mt_json_get_field_required(json_topic, "name", MT_JSONFieldType_String);
    if (json_name == NULL)
        goto err;
    topic->name = g_strdup(cJSON_GetStringValue(json_name));

    const cJSON *const json_tags = mt_json_get_field_required(json_topic, "tags", MT_JSONFieldType_String);
    if (json_tags == NULL)
        goto err;
    topic->equation = mt_equation_compile(cJSON_GetStringValue(json_tags), tag_storage);
    if (topic->equation == NULL) {
        mt_errorf("Equation compilation error");
        goto err;
    }

    const cJSON *const json_gather_period = mt_json_get_field_required(json_topic, "gather_period", MT_JSONFieldType_Number);
    if (json_gather_period == NULL)
        goto err;
    topic->gather_period = cJSON_GetNumberValue(json_gather_period);
    if (topic->gather_period < 100.0) {
        mt_warnf("Gather period is too small: clamped to 100ms");
        topic->gather_period = 100.0;
    }
    topic->gather_timer = g_timer_new();
    topic->entry_list = mt_entry_list_new(tag_storage);

    return topic;
err:
    mt_topic_free(topic);
    return NULL;
}

static bool mt_topics_init(const cJSON *const json_parent, GPtrArray *const topics, MT_TagStorage *const tag_storage)
{
    const cJSON *const json_topics = mt_json_get_field_required(json_parent, "topics", MT_JSONFieldType_Array);
    if (json_topics == NULL)
        return false;
    const size_t num_topics = cJSON_GetArraySize(json_topics);
    for (size_t i = 0; i < num_topics; ++i) {
        const cJSON *const json_topic = mt_json_get_array_item(json_topics, i, MT_JSONFieldType_Object);
        MT_Topic *topic = mt_topic_new(json_topic, tag_storage);
        if (topic == NULL)
            return false;
        g_ptr_array_add(topics, topic);
    }
    return true;
}

MT_Client *mt_client_new(const char *const json_contents)
{
    MT_Client *client = g_new0(MT_Client, 1);
    // FIXME: magic constant
    client->tag_storage = mt_tag_storage_new(1024);

    cJSON *json = cJSON_Parse(json_contents);
    if (json == NULL) {
        mt_errorf("Error parsing JSON config: %s", cJSON_GetErrorPtr());
        goto err_parsing;
    }

    const cJSON *const json_broker = mt_json_get_field_required(json, "broker", MT_JSONFieldType_Object);
    if (json_broker == NULL)
        goto err_semantic;
    if (!mt_broker_info_init(json_broker, &client->broker_info)) {
        mt_notef("At: \"%s\"", "broker");
        goto err_semantic;
    }

    client->modules = g_ptr_array_new();
    if (!mt_modules_load(json, client->modules, client->tag_storage))
        goto err_semantic;

    client->topics = g_ptr_array_new();
    if (!mt_topics_init(json, client->topics, client->tag_storage))
        goto err_semantic;

    mosquitto_lib_init();
    client->mosquitto = mosquitto_new(client->broker_info.client_id, client->broker_info.clean_session, NULL);
    if (client->mosquitto == NULL) {
        mt_errorf("Error creating Mosquitto instance");
        goto err_mosquitto;
    }

    cJSON_Delete(json);
    return client;

err_mosquitto:
err_semantic:
    cJSON_Delete(json);
err_parsing:
    mt_client_destroy(client);
    return NULL;
}

void mt_client_run(MT_Client *const client)
{
    // FIXME: magic constants
    int status = mosquitto_connect(client->mosquitto, client->broker_info.address, client->broker_info.port, 60);
    if (status != MOSQ_ERR_SUCCESS) {
        mt_errorf("Connection error: %s", mosquitto_strerror(status));
        mt_notef("Check if broker is running");
        return;
    }

    for (size_t i = 0; i < client->topics->len; ++i) {
        const MT_Topic *const topic = g_ptr_array_index(client->topics, i);
        g_timer_start(topic->gather_timer);
    }

    mosquitto_loop_start(client->mosquitto);
    while (true) {
        for (size_t topic_i = 0; topic_i < client->topics->len; ++topic_i) {
            const MT_Topic *const topic = g_ptr_array_index(client->topics, topic_i);
            if (g_timer_elapsed(topic->gather_timer, NULL) * 1000.0 >= topic->gather_period) {
                topic->entry_list->topic.equation = topic->equation;
                cJSON *result_json = cJSON_CreateObject();
                g_timer_reset(topic->gather_timer);
                for (size_t module_i = 0; module_i < client->modules->len; ++module_i) {
                    const MT_Module *const module = g_ptr_array_index(client->modules, module_i);
                    topic->entry_list->entry.module = module->name;
                    topic->entry_list->topic.json = cJSON_CreateObject();
                    mt_tag_set_clear(topic->entry_list->entry.tags);
                    module->gather(topic->entry_list, module->module_data);
                    cJSON_AddItemToObject(result_json, module->name, topic->entry_list->topic.json);
                }

                char *const result_json_text = cJSON_PrintUnformatted(result_json);

                int status = mosquitto_publish(client->mosquitto, NULL, topic->name, strlen(result_json_text) + 1, result_json_text, 1, false);
                cJSON_Delete(result_json);
                cJSON_free(result_json_text);
                if (status != MOSQ_ERR_SUCCESS) {
                    mt_errorf("Error publishing: %s", mosquitto_strerror(status));
                    return;
                }
            }
        }
        // TODO: subtract gathering time
        // FIXME: magic constants
        usleep(10000);
    }
}

void mt_client_destroy(MT_Client *const client)
{
    if (client == NULL)
        return;
    mt_broker_info_free(&client->broker_info);

    if (client->topics != NULL)
        for (size_t i = 0; i < client->topics->len; ++i)
            mt_topic_free(g_ptr_array_index(client->topics, i));
    g_ptr_array_free(client->topics, TRUE);

    if (client->modules != NULL)
        for (size_t i = 0; i < client->modules->len; ++i)
            mt_module_unload(g_ptr_array_index(client->modules, i));
    g_ptr_array_free(client->modules, TRUE);

    mt_tag_storage_destroy(client->tag_storage);

    // mosquitto_disconnect(client->mosquitto);
    mosquitto_destroy(client->mosquitto);
    mosquitto_lib_cleanup();

    g_free(client);
}
