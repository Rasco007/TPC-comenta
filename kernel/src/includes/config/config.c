#include <includes/config/config.h>

tKernelConfig* armarKernelConfig(t_config* config);
tAlgoritmosPlanificacion charTotAlgoritmosPlanificacion(char* givenChar);
void add_to_list(char **source, t_list *destiny);
void destruirMemoriaDinamica(t_config* config, char** recursos, char** instanciasRecursos);
void free_arr(char **arr);

tKernelConfig* leerKernelConfig(t_log *logger) {

    t_config* config = leerConfig(kernelConfigPath, logger);

    if (config == NULL) {
        return config;
    }

    tKernelConfig* kernelConfig = armarKernelConfig(config);

    log_debug(logger, "Archivo de configuracion %s leido correctamente", kernelConfigPath);

    return kernelConfig;
}

tKernelConfig* armarKernelConfig(t_config* config) {
    
    tKernelConfig* kernelConfig = malloc(sizeof(tKernelConfig));

    //configs a nivel cliente y servidor
    kernelConfig->puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    kernelConfig->ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    kernelConfig->puertoMemoria = config_get_string_value(config, "PUERTO_MEMORIA");
    kernelConfig->ipCpu = config_get_string_value(config, "IP_CPU");
    kernelConfig->puertoCpuDispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    kernelConfig->puertoCpuInterrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    //configs a nivel planificacion
    char* configAlgoritmoPlanificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    tAlgoritmosPlanificacion mappedAlgoritmoPlanificacion = charTotAlgoritmosPlanificacion(configAlgoritmoPlanificacion);
    
    kernelConfig->algoritmoPlanificacion = mappedAlgoritmoPlanificacion;
    kernelConfig->quantum = config_get_int_value(config, "QUANTUM");

    //configs a nivel recursos e instancias
    kernelConfig->recursos = list_create();
    kernelConfig->instanciasRecursos = list_create();
    char **recursos = config_get_array_value(config, "RECURSOS");
    char **instanciasRecursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    add_to_list(recursos, kernelConfig->recursos);
    add_to_list(instanciasRecursos, kernelConfig->instanciasRecursos);
    kernelConfig->gradoMultiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

    destruirMemoriaDinamica(config, recursos, instanciasRecursos);

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

void destruirMemoriaDinamica(t_config* config, char** recursos, char** instanciasRecursos) {

    config_destroy(config);
    free_arr(recursos);
    free_arr(instanciasRecursos);
}

void add_to_list(char **source, t_list *destiny){

    int i = 0;
    while (source[i] != NULL)
    {
        list_add(destiny, source[i]);
        i++;
    }
}

void free_arr(char **arr){
    
    int i = 0;
    while (arr[i] != NULL)
    {
        free(arr[i]);
        i++;
    }
    free(arr);
}