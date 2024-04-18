#ifndef LOGGER_H_
#define LOGGER_H_

#include<commons/log.h>
#include<shared/init.h>

static const char* interfazLoggerFilePath = "entradasalida_log.log";
static const char* interfazModuleName = "entradasalida.c";

t_log* iniciarInterfazLogger();

#endif /* LOGGER_H_ */ 