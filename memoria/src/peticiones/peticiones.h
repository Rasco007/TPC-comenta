#ifndef PETICION_MEMORIA_H

 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <estructura/estructura.h>
#include <conexionKernel/conexionKernel.h>


extern int sockets[3]; 
extern t_log* logger; 
extern t_log* loggerError; 
extern t_config* config; 

#endif