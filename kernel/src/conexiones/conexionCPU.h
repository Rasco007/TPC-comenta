#ifndef CONEXION_CPU_H
#define CONEXION_CPU_H

#include <commons/config.h>
#include <commons/string.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <global.h>

extern int conexionACPU;
extern t_log* loggerError; 
extern t_contexto* contextoEjecucion;


void conexionCPU();



// FUNCIONES PARA ENVIO DE CONTEXTO DE EJECUCION

t_dictionary *registrosDelCPU(t_dictionary *aCopiar);

t_dictionary *registrosDelCPU(t_dictionary *aCopiar);


#endif /* CONEXION_CPU_H */