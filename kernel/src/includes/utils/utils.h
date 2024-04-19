#ifndef UTILS_H_
#define UTILS_H_

#include <includes/config/config.h>
#include <client_handler/client_handler.h>
#include <stdlib.h>

void terminarPrograma(tKernelConfig* kernelConfig, t_log* kernelLogger);
void liberarLogger(t_log* kernelLogger);

#endif