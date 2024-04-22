#ifndef IO_MEM_SER_H
#define IO_MEM_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>

#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <contextoEjecucion/contextoEjecucion.h>

extern t_log* loggerError;

void conexionMemoria(); 

#endif