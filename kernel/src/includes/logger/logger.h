#ifndef LOGGER_H_
#define LOGGER_H_

#include<commons/log.h>
#include<shared/init.h>

static const char* kernelLoggerFilePath = "kernel_log.log";
static const char* kernelModuleName = "kernel.c";

t_log* iniciarKernelLogger();

#endif /* LOGGER_H_ */