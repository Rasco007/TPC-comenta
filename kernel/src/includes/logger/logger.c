#include <includes/logger/logger.h>

t_log* iniciarKernelLogger()
{
	t_log* nuevo_logger = log_create(kernelLoggerFilePath, kernelModuleName, true, LOG_LEVEL_INFO);

	return nuevo_logger;
}