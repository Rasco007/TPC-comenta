#ifndef CONEXION_IO_H
#define CONEXION_IO_H

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>

void conexionIO();

extern t_log* logger; 
extern t_log* loggerError; 

#endif
