#include <includes/logger/logger.h>

t_log* iniciarMemoriaLogger(){
	t_log* nuevo_logger = iniciarLogger(memoriaLoggerFilePath, memoriaModuleName, true, LOG_LEVEL_INFO);
	return nuevo_logger;
}