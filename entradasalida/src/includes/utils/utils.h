#ifndef UTILS_H_
#define UTILS_H_

#include <includes/config/config.h>
#include <client_handler/client_handler.h>
#include <stdlib.h>

void terminarPrograma(tInterfazConfig* interfazConfig, t_log* interfazLogger);
void liberarLogger(t_log* interfazLogger);

#endif /* UTILS_H_ */