#ifndef MEMORIA_CONEXION_KERNEL_H
#define MEMORIA_CONEXION_KERNEL_H

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <estructura/estructura.h>
#include <global.h>
#include <main/memoria.h>

// Variables globales
//extern t_log *logger;
//extern t_config *config;
extern MemoriaFisica *mf;
extern char* pathInstrucciones;
extern int PID;

// Funciones
int ejecutarServidorKernel(int *socketCliente);
void eliminarProcesoDeMemoria(int pid);
Proceso *buscar_proceso_por_pid(int pid);
#endif // MEMORIA_CONEXION_KERNEL_H
