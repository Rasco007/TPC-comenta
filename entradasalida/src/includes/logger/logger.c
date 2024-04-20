#include <includes/logger/logger.h>

t_log* iniciarInterfazLogger()
{
	t_log* nuevo_logger = iniciarLogger(interfazLoggerFilePath, interfazModuleName, true, LOG_LEVEL_INFO);
	return nuevo_logger;
} 