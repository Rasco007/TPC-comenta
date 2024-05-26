/* - Interprete entre el hardware y el software - */
#ifndef KERNEL_H_
#define KERNEL_H_

// Generales
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>

// Conexiones propias del kernel
#include <conexiones/conexionMemoria.h>
#include <conexiones/conexionCPU.h>
#include <main/configuraciones.h>
#include <escuchaIO/servidorIO.h>

// Utils y funciones del shared
#include <configuraciones/configuraciones.h>

extern int socketCliente;
extern t_log* logger;
extern t_log* loggerError;
extern t_config* config;
extern pthread_t planificadorLargoPlazo_h, planificadorCortoPlazo_h, recibirConsolas_h;

#endif