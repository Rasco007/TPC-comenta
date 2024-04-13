#include <includes/config/config.h>

void add_to_list(char **source, t_list *destiny);
void free_arr(char **arr);
tAlgoritmosPlanificacion charTotAlgoritmosPlanificacion(char* givenChar);

tKernelConfig* leerKernelConfig(t_log *logger) {

    t_config* config = config_create(kernelConfigPath);
    
    if (config == NULL) {
        log_error(logger, "la config no existe");
        return NULL;
    }

    tKernelConfig* kernelConfig = malloc(sizeof(tKernelConfig));

    //configs a nivel cliente y servidor
    kernelConfig->ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    kernelConfig->puertoMemoria = config_get_string_value(config, "PUERTO_MEMORIA");
    kernelConfig->ipCpu = config_get_string_value(config, "IP_CPU");
    kernelConfig->puertoCpuDispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    kernelConfig->puertoCpuInterrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    //configs a nivel planificacion
    char* configAlgoritmoPlanificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    tAlgoritmosPlanificacion mappedAlgoritmoPlanificacion = charTotAlgoritmosPlanificacion(configAlgoritmoPlanificacion);
    
    kernelConfig->algoritmoPlanificacion = mappedAlgoritmoPlanificacion;
    kernelConfig->algoritmoPlanificacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    kernelConfig->quantum = config_get_int_value(config, "QUANTUM");

    kernelConfig->quantum = config_get_int_value(config, "QUANTUM");

    kernelConfig->recursos = list_create();
    kernelConfig->instanciasRecursos = list_create();
    char **recursos = config_get_array_value(config, "RECURSOS");
    char **instanciasRecursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    add_to_list(recursos, kernelConfig->recursos);
    add_to_list(instanciasRecursos, kernelConfig->instanciasRecursos);
    log_debug(logger, "Archivo de configuracion leido correctamente");

    config_destroy(config);
    free_arr(recursos);
    free_arr(instanciasRecursos);

    return kernelConfig;
}

tAlgoritmosPlanificacion charTotAlgoritmosPlanificacion(char* givenChar) {
     if (strcmp(givenChar, "VRR") == 0) {
          return VRR;
     }

     if (strcmp(givenChar, "FIFO") == 0) {
          return FIFO;
     }

     return UNKNOWN;
}

void add_to_list(char **source, t_list *destiny)
{
    int i = 0;
    while (source[i] != NULL)
    {
        list_add(destiny, source[i]);
        i++;
    }
}

void free_arr(char **arr)
{
    int i = 0;
    while (arr[i] != NULL)
    {
        free(arr[i]);
        i++;
    }
    free(arr);
}