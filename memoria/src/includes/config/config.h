#ifndef MEMORIA_CONFIG_H_
#define MEMORIA_CONFIG_H_

#include <commons/collections/list.h>
#include <shared/structs.h>
#include <shared/init.h>

static const char* memoriaConfigPath = "./configs/memoria.config";

typedef struct{
    char* puertoEscucha;
    int tamMemoria;
    int tamPagina;
    char* pathInstrucciones;
    int retardoRespuesta;
} tMemoriaConfig;

tMemoriaConfig* leerMemoriaConfig(t_log *logger);

#endif