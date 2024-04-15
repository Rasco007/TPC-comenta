#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

#include <commons/collections/list.h>
#include <shared/structs.h>
#include <shared/init.h>

static const char* kernelConfigPath = "./configs/kernel.config";

typedef struct
{
    int puertoEscucha;
    char* ipMemoria;
    int puertoMemoria;
    char* ipCpu;
    int puertoCpuDispatch;
    int puertoCpuInterrupt;
    tAlgoritmosPlanificacion algoritmoPlanificacion;
    int quantum;
    t_list* recursos;
    t_list* instanciasRecursos;
    int gradoMultiprogramacion;
} tKernelConfig;

tKernelConfig* leerKernelConfig(t_log *logger);

#endif /* KERNEL_CONFIG_H_ */