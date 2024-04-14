#include <includes/config/config.h>

tCPUconfig* leerConfig(t_log *logger){
    t_config* config=config_create(CPUConfigPath);
    
    if (config == NULL) {
        log_error(logger, "la config no existe");
        return NULL;
    }

    tCPUconfig* cpuConfig=malloc(sizeof(tCPUconfig));

    cpuConfig->ipMemoria=config_get_string_value(config,"IP_MEMORIA");
    cpuConfig->puertoMemoria=config_get_int_value(config,"PUERTO_MEMORIA");
    cpuConfig->puertoEscuchaDispatch=config_get_int_value(config,"PUERTO_ESCUCHA_DISPATCH");
    cpuConfig->puertoEscuchaInterrupt=config_get_int_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    cpuConfig->entradasTLB=config_get_int_value(config,"CANTIDAD_ENTRADAS_TLB");
    cpuConfig->algoritmoTLB=config_get_string_value(config,"ALGORITMO_TLB");

    log_debug(logger,"Archivo de configuracion leido correctamente");
    config_destroy(config);
    return cpuConfig;
}