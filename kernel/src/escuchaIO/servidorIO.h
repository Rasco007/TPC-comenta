#ifndef CPU_KERN_SER_H
#define CPU_KERN_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>

#include <./main/kernel.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <contextoEjecucion/contextoEjecucion.h>

extern t_log* loggerError; 
typedef struct {
    char nombre_interfaz[256];
    char tipo_interfaz[256];
    int socket_interfaz;
} Interfaz;

// Estructura para gestionar el arreglo dinámico de interfaces
typedef struct {
    Interfaz *interfaces;
    size_t cantidad;
} Kernel_io;

void escucharAlIO(); 
void destruirStructsIO(Kernel_io *kernel);
void inicializarStructsIO(Kernel_io *kernel);
int existeLaInterfaz(char *nombreInterfaz, const Kernel_io *kernel);
int verificarConexionInterfaz(Kernel_io *kernel, const char *nombre_interfaz);
int obtener_socket(const Kernel_io *kernel, const char *nombre_interfaz);
extern t_contexto* contextoEjecucion;

extern Kernel_io kernel;
#endif