#include <includes/logger/logger.h>

t_log* iniciarCPULogger(){
	t_log* nuevo_logger = log_create(CPULoggerFilePath, CPUModuleName, true, LOG_LEVEL_INFO);
	return nuevo_logger;
}