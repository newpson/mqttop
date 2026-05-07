# mqttop

MQTT system monitoring tool. A small utility that can be used to monitor any parameters of system software or hardware (Linux-only), sending all data via MQTT.

## Build & use
```
mkdir build
cd build
cmake ../
make
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:./modules" ./mqttop ../config.json
```

# Detailed description

## Basic data structures

C language does not have generics, so the only way to create a polymorphic data structure is to use the void pointers. It forces you to split data storage and representation. [Glib](https://docs.gtk.org/glib/) library provides a great amount of different modules and data structures made with this approach. In this project most used are:

* `GArray` (an array of blocks with known size) and `GPtrArray` (an array of void pointers);

* `GStringChunk` (a fragmented storage of linked blocks of strings; string addresses are constant during storage growth);

* `GHashTable` (classical "void-polymorphic" hash table operating with specified hashing and comparing functions; does not own the data).

## Basic concepts (high-level design)

After you run the client, first thing first it parses JSON configuration file. JSON is used to minimize the number of dependencies in the project, so don't be mad, it has really simple structure:
```json
{
  "broker": {
    "client_id": "mqttop-001",
    "address": "127.0.0.1",
    "port": 1883,
    "clean_session": false
  },
  "modules": ["mod_meminfo.so", "mod_cpufreq.so"],
  "topics": [
    {
      "name": "Memory",
      "tags": "MemFree | MemAvailable | Cached | SwapFree",
      "gather_period": 2500
    },
    {
      "name": "CPU",
      "tags": "cpu",
      "gather_period": 1000
    }
  ]
}
```
As you can see in the example above, there are only three root items:

* `broker` - currently there is no support of multiple brokers as well as password authentication, so you specify just a few settings for only one broker;

* `modules` - the list of modules that will be dynamically loaded using `<dlfcn.h>` API. The module name can't be aliased currently. Every module has a very simple structure that will be described just below;

* `topics` - the list of topics that the client will publish upon the specified periods of time. Every topic has it's name (you can implement classical MQTT path/channel hierarchical structure), data publish/gather period in milliseconds and some tag equation that will be described later too.
  
After the parsing of the config (and loading the modules and initializing the topics) the client connects to the broker and starts a timer-based topic data gathering. Every topic has its own gathering period, and when the time becomes, the main client goes through all the modules and filters their entries to match the topic tag equation, generating the JSON object. The resulting JSON object is sent to the broker and then the client waits for the next timer.

### Modules

The main client acts as functional proxy between raw data and MQTT broker. Raw data can be collected in any way: procedural generation, file reading, network interaction (synchronous) as long as you can insert a couple of simple API calls between the actual data collection and your some complicated logic.

Every module must have at least three functions to become a *module*:

* `void *mt_module_init()` - this one may initialize or allocate some data for further usage in data collection phase if needed, called once upon the module loading;

* `void mt_module_free(void *)` - the opposite of `mt_module_init()`, called once when the client terminates;

* `void mt_module_gather(MT_EntryList *, void *)` - this is where the "simple API" lives, called at least on once per topic publishment; the goal is to generate so-called *entries* (key-value pairs) for every data piece you want to publish:

  * `void mt_entry_begin(MT_EntryList *, const char *id)`  
    First of all, you create a new entry in the entry list, a unique string allows the client to indentify the entry among others in the same module.
    
  * `void mt_entry_push_tag(MT_EntryList *, const char *id)`  
    If you want to give the user more different ways to filter the data provided by your module, you can attach tags to your entries (more = better). By default, an entry has only two tags attached: its id and module name.

  * `bool mt_entry_is_suitable(MT_EntryList *)`  
    Before putting a specific value into the entry, you must ensure that the main client will include that entry in the publishment. The client will perform tag filtering and report whether the entry should be discarded or not (you can set the value without filtering if you have good reasons).
    
  * `void mt_entry_set_value_null(MT_EntryList *)`
    `void mt_entry_set_value_bool(MT_EntryList *, bool)`
    `void mt_entry_set_value_number(MT_EntryList *, double)`
    `void mt_entry_set_value_string(MT_EntryList *, const char *)`
    This group of functions allows you to set the value of the entry. The type of the value is [JSON-compatible](https://www.w3schools.com/js/js_json_datatypes.asp). After you call any of these functions the entry will be transmuted to JSON data, so you can only create a new entry then.

### Topics

An example of topic publishment:
```json
// (Topic: Memory, QoS: 1)
{
  "mod_meminfo.so": {
    "MemFree": 1131924,
    "MemAvailable": 2952464,
    "Cached": 2628856,
    "SwapFree": 7384028
  },
  "mod_cpufreq.so": {}
}
```
Every topic represents a list of filtered module outputs. Even if the module didn't return any suitable data for the publishment, its item will be shown anyway (as empty object).

### Tag equations

The top-deal feature of this specific monitoring tool is the tag-based entry filtering system. As already said, every tag is just a string that somehow describes or generalizes the semantic meaning of the entry value. For example, if your module returns some CPU temperature values for different cores, but their IDs are not informative enough to act as group names (`cpu0`, `cpu1`, etc), you can add some tags like `hardware`, `cpu`, `temperature` to all the entries for flexible data filtering. And now, if we want, for example, to collect information from various temperature sensors, we can just put `temperature` tag in the configuration file and get all the entries with this tag from different modules.

To create a new topic it's enough to specify some classic boolean equation with NOT, OR, AND operations plus parentheses to change operations priority. Tags do act as variables in this sort of equation. The compilation of the equation produces a hash set of all the tags in that equaiton. To evaluate the equation, you provide it with another so-called *presence hash set* where you put only those tags that you want to become boolean true (other tags will represent boolean false then). And the result of the evaluation can be interpreted as "this set of tags satisfy the equation" (i.e. the entry should be included in the topic publishment).

## MQTTOP Module API usage examples

* Easy: RAM usage information (`meminfo`)  
  The module just parses `/proc/meminfo` turning each line into an entry without attaching any specific tags.

* Medium: CPU usage information (`cpufreq`)  
  The module searches through `/sys/bus/cpu/devices` (not really deep), dynamically generating IDs and tags for each entry using file and directory names.

* Advanced (TODO): hardware monitoring sensors (`hwmon`)  
  The module goes through `/sys/class/hwmon`, dynamically generating IDs and tags for each entry based on the `sysfs` organization (`realpath`, `readlink` are used). More detailed information on how this module collects data can be found [here](https://docs.kernel.org/hwmon/sysfs-interface.html).