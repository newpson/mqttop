#include "mt_module.h"
#include "mt_logf.h"
#include <dlfcn.h>
#include <glib.h>
#include <stddef.h>

MT_Module *mt_module_load(const char *const path)
{
    MT_Module *module = g_new0(MT_Module, 1);

    module->so_handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (module->so_handle == NULL) {
        mt_errorf("Unable to load module \"%s\": %s", path, dlerror());
        goto err;
    }

    module->init = (MT_Module_InitFunPtr)dlsym(module->so_handle, "mt_module_init");
    if (module->init == NULL) {
        mt_errorf("Unable to load symbol \"%s\": %s", "mt_module_init", dlerror());
        goto err;
    }

    module->gather = (MT_Module_GatherFunPtr)dlsym(module->so_handle, "mt_module_gather");
    if (module->gather == NULL) {
        mt_errorf("Unable to load symbol \"%s\": %s", "mt_module_gather", dlerror());
        goto err;
    }

    module->free = (MT_Module_FreeFunPtr)dlsym(module->so_handle, "mt_module_free");
    if (module->free == NULL) {
        mt_errorf("Unable to load symbol \"%s\": %s", "mt_module_free", dlerror());
        goto err;
    }

    module->name = path;

    return module;
err:
    mt_module_unload(module);
    return NULL;
}

void mt_module_unload(MT_Module *const module)
{
    if (module == NULL)
        return;
    if (module->free != NULL)
        module->free(module->module_data);
    if (module->so_handle != NULL)
        dlclose(module->so_handle);
    g_free(module);
}