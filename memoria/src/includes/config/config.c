#include <includes/config/config.h>

tMemoriaConfig *leerMemoriaConfig(t_log *logger);
tMemoriaConfig *armarMemoriaConfig(t_config *config);

tMemoriaConfig *leerMemoriaConfig(t_log *logger){

    t_config *config = leerConfig(memoriaConfigPath, logger);

    if (config == NULL){
        return config;
    }

    tMemoriaConfig *memoriaConfig = armarMemoriaConfig(config);

    log_debug(logger, "Archivo de configuracion %s leido correctamente", memoriaConfigPath);

    return memoriaConfig;
}

tMemoriaConfig *armarMemoriaConfig(t_config *config){

    tMemoriaConfig *memoriaConfig = malloc(sizeof(tMemoriaConfig));

    memoriaConfig->puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    memoriaConfig->tamMemoria = config_get_string_value(config, "TAM_MEMORIA");
    memoriaConfig->tamPagina = config_get_string_value(config, "TAM_PAGINA");
    memoriaConfig->pathInstrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    memoriaConfig->retardoRespuesta = config_get_string_value(config, "RETARDO_RESPUESTA");

    return memoriaConfig;
}