#ifndef IO_MAIN_H_
#define IO_MAIN_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

#include <conexionKernel/conexionKernel.h>
#include <conexionMemoria/conexionMemoria.h>
#include "../include/io_globals.h"

#include <global.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <configuraciones/configuraciones.h>

#include "../include/iniciar_io.h"



void iniciar_io(int argc, char** argv);


//Var globales
t_log* io_logger;
t_log* io_loggerError;
t_config* io_config;

int fd_kernel;
int fd_memoria;

char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;

#endif
