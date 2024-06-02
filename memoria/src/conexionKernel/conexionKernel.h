#ifndef MEMORIA_CONEXION_KERNEL_H
#define MEMORIA_CONEXION_KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <estructura/estructura.h>
#include <global.h>
//#include <configuraciones/configuraciones.h>

extern t_config* config;
extern t_log* logger;
extern int sockets[3];

int ejecutarServidorKernel(int*);
t_proceso *crearProcesoEnMemoria(int pid);

#endif