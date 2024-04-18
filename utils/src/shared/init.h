#ifndef INIT_H_
#define INIT_H_

#include<commons/config.h>
#include<commons/log.h>

t_config* leerConfig(char* path, t_log* logger);
t_log* iniciarLogger(char* loggerFilePath, char* moduleName, bool isActiveConsole, t_log_level logLevel);

#endif /* INIT_H_ */ 