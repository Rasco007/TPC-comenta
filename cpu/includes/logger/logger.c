#include <includes/logger/logger.h>

t_log* iniciarCPULogger(){
	t_log* nuevo_logger = log_create(CPULoggerFilePath, CPUModuleName, true, LOG_LEVEL_INFO);
    if(nuevo_logger == NULL) {
		perror("No se pudo crear el logger");
		exit(1);
	}

	return nuevo_logger;
}