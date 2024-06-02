#ifndef MEMORIA_CONEXION_CPU_H
#define MEMORIA_CONEXION_CPU_H

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <global.h>
#include <main/memoria.h>
#include <estructura/estructura.h>
//#include <configuraciones/configuraciones.h>

int ejecutarServidorCPU(int*); 

extern void* espacioDeUsuario; 
extern Memoria* memoria;


extern t_config* config;
extern t_log* logger; 
extern t_log* loggerError; 


#endif 



