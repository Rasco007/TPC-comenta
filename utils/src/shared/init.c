#include <shared/init.h>

t_config* leerConfig(char* path, t_log *logger) {

    t_config* config = config_create(path);

    if (config == NULL) {
        log_error(logger, "La config %s no existe", path);
        return NULL;
    }

    return config;
}

t_log* iniciarLogger(char* loggerFilePath, char* moduleName, bool isActiveConsole, t_log_level logLevel) {

    t_log* nuevo_logger = log_create(loggerFilePath, moduleName, isActiveConsole, logLevel);
	return nuevo_logger;
} 