#ifndef LOGGER_H_
#define LOGGER_H_

#include<commons/log.h>
#include<shared/init.h>

static const char* CPULoggerFilePath = "cpu_log.log";
static const char* CPUModuleName = "cpu.c";

t_log* iniciarCPULogger();

#endif /* LOGGER_H_ */