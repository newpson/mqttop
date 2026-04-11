#include <mosquitto.h>
#include <stddef.h>

int main(void)
{
    mosquitto_lib_init();

    const char my_data[] = "important data 123";

    struct mosquitto *mosq = mosquitto_new("my_mosquitto", false, NULL);
    mosquitto_connect(mosq, "127.0.0.1", 1883, 3600);
    mosquitto_publish(mosq, NULL, "my_topic", sizeof(my_data), (void *)my_data, 0, false);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();
    return 0;
}