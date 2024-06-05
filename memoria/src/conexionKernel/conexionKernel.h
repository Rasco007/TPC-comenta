#ifndef MEMORIA_CONEXION_KERNEL_H
#define MEMORIA_CONEXION_KERNEL_H

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <estructura/estructura.h>
#include <global.h>

// Definiciones de las operaciones
#define NEWPCB 0
#define ENDPCB 1

// Variables globales
extern t_log *logger;
extern t_config *config;
extern MemoriaFisica *memoria;

// Funciones
int ejecutarServidorKernel(int *socketCliente);
Proceso *crearProcesoEnMemoria(int pid);
void eliminarProcesoDeMemoria(int pid);

#endif // MEMORIA_CONEXION_KERNEL_H
