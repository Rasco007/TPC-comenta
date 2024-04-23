#ifndef MEMORIA_CONEXION_CPU_H
#define MEMORIA_CONEXION_CPU_H

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <global.h>

int ejecutarServidorIO(int*); 

extern t_config* config;
extern t_log* logger; 
extern t_log* loggerError; 


#endif 



