#ifndef CPU_CONFIG_H_
#define CPU_CONFIG_H_

#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <shared/structs.h>
#include <shared/init.h>

static const char* CPUConfigPath = "./configs/cpu.config";

typedef struct{
    char* ipMemoria;
    int puertoMemoria;
    int puertoEscuchaDispatch;
    int puertoEscuchaInterrupt;
    int entradasTLB;
    char* algoritmoTLB;
} tCPUconfig;

tCPUconfig* leerCPUConfig(t_log *logger);

#endif /* CPU_CONFIG_H_ */