#include <includes/config/config.h>

tInterfazConfig* armarInterfazConfig(t_config* config);
tInterfaces charTotInterfaces(char* givenChar);
void add_to_list(char **source, t_list *destiny);
void free_arr(char **arr);

tInterfazConfig* leerInterfazConfig(t_log *logger) {

    t_config* config = leerConfig(interfazConfigPath, logger);

    if (config == NULL) {
        return config;
    }

    tInterfazConfig* interfazConfig = armarInterfazConfig(config);

    log_debug(logger, "Archivo de configuracion %s leido correctamente", interfazConfigPath);

    return interfazConfig;
}

tInterfazConfig* armarInterfazConfig(t_config* config) {

    tInterfazConfig* interfazConfig = malloc(sizeof(tInterfazConfig));

    //configs a nivel interfaz
    char* configTipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    tInterfaces mappedTipoInterfaz = charTotInterfaces(configTipoInterfaz);
    interfazConfig->tipo_interfaz = mappedTipoInterfaz;


    interfazConfig->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    //configs a nivel cliente y servidor
    interfazConfig->ip_kernel= config_get_string_value(config, "IP_KERNEL");
    interfazConfig->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    interfazConfig->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    interfazConfig->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //config path
    //TODO - ¿Tendré que modificar el tipo?
    interfazConfig->path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");

    //config bloques
    interfazConfig->block_size = config_get_string_value(config, "BLOCK_SIZE");
    interfazConfig->block_count = config_get_string_value(config, "BLOCK_COUNT");


    return interfazConfig;
}

tInterfaces charTotInterfaces(char* givenChar) {

     if (strcmp(givenChar, "STDOUT") == 0) {
          return STDOUT;
     }

     if (strcmp(givenChar, "STDIN") == 0) {
          return STDIN;
     }
      if (strcmp(givenChar, "DIALFS") == 0) {
          return DIALFS;
     }

     return UNKNOWN;
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