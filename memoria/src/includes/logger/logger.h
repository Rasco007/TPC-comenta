#ifndef LOGGER_H_
#define LOGGER_H_

#include<commons/log.h>
#include<shared/init.h>

static const char* memoriaLoggerFilePath = "memoria_log.log";
static const char* memoriaModuleName = "memoria.c";

t_log* iniciarMemoriaLogger();

#endif