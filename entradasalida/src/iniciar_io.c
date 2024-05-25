#include <../include/iniciar_io.h>

void iniciar_io(int argc, char** argv){
    iniciar_logs();
    iniciar_configs(argv[1]);
    mostrar_configs();
}

void iniciar_logs(){
    io_logger = log_create("entradasalida.log", "LOGGER_IO", true, LOG_LEVEL_INFO);
	if( io_logger == NULL){
		perror ("error al crear log");
		exit(EXIT_FAILURE);
	}else{
        log_info (io_logger, "[IO] Log iniciado correctamente");
    }

    io_loggerError = log_create("errores.log", "LOGGER__ERROR_IO", true, LOG_LEVEL_INFO);
	if( io_loggerError == NULL){
		perror ("error al crear log error");
		exit(EXIT_FAILURE);
	}else{
        log_info (io_loggerError, "[IO] Error Log inciado correctamente");
    }


}

void iniciar_configs(char* arg){
    io_config = config_create(arg);
    
    if (io_config == NULL) {
        // No se pudo crear el config, terminemos el programa
		perror("Error al intentar cargar el config.");
        exit(EXIT_FAILURE);
    }else{
        log_info (io_logger, "[IO] Configuracion obtenida correctamente");
    }
    TIPO_INTERFAZ = config_get_string_value(io_config, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = config_get_int_value(io_config, "TIEMPO_UNIDAD_TRABAJO");
    IP_KERNEL = config_get_string_value(io_config, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(io_config, "PUERTO_KERNEL");
    IP_MEMORIA = config_get_string_value(io_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(io_config, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(io_config, "PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(io_config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(io_config, "BLOCK_COUNT");
}

void mostrar_configs(){

    log_info(io_logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);
    log_info(io_logger, "TIEMPO_UNIDAD_TRABAJO: %d", TIEMPO_UNIDAD_TRABAJO);
    log_info(io_logger, "IP_KERNEL: %s", IP_KERNEL);
    log_info(io_logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    log_info(io_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(io_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(io_logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    log_info(io_logger, "BLOCK_SIZE: %d", BLOCK_SIZE);
    log_info(io_logger, "BLOCK_COUNT: %d", BLOCK_COUNT);
}