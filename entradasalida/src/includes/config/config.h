#ifndef INTERFAZ_CONFIG_H_
#define INTERFAZ_CONFIG_H_

#include <commons/collections/list.h>
#include <shared/structs.h>
#include <shared/init.h>


static const char* interfazConfigPath = "./configs/interfaz.config";

typedef struct
{
    tInterfaces tipo_interfaz;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
    char* path_base_dialfs;
    int block_size;
    int block_count;
   
} tInterfazConfig;

tInterfazConfig* leerInterfazConfig(t_log *logger);

#endif /* INTERFAZ_CONFIG_H_ */ 