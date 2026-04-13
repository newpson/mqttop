# MQTTTop
MQTT system monitoring tool.

A small utility that can be used to monitor any parameters of system software or hardware (Linux-only), sending all data via MQTT.

## Features

### Application
- [ ] Temperature/cooling sensors:  
    `/sys/class/thermal/thermal_zone##`, `/sys/class/thermal/cooling_device##`;
- [ ] RAM usage:  
    `struct sysinfo`, `/proc/meminfo` (`MemAvailable` can only be get with `si_mem_available()` kernel-private API?);
- [ ] CPU usage:  
    `struct sysinfo`, `/sys/bus/cpu/devices/cpu##/cpufreq/scaling_cur_freq`;
- [ ] Custom metrics (specific path);
- [ ] Custom period for every metric;
- [ ] Configuration through MQTT;
- [ ] Automatic sensor searching:  
      the indicies in the hardcoded paths will be iterated until ENOENT;
- [ ] Configuration files (INI or YAML or TOML? not JSON).

### Code base
- [ ] Simple architecture (easily debuggable);
- [ ] Configurable logging system;
- [ ] Tests;
- [ ] Doc comments.