#ifndef MEMORIA_MAIN_H_
#define MEMORIA_MAIN_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

#include <conexionKernel/conexionKernel.h>
#include <conexionCPU/conexionCPU.h>
#include <conexionIO/conexionIO.h>
#include <semaphore.h>
#include <estructura/estructura.h> 

#include <estructura/estructura.h> 

#include <global.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <configuraciones/configuraciones.h>


extern t_log* loggerError; 
extern sem_t path;
extern int sockets[2];
extern int server_fd;
#endif
