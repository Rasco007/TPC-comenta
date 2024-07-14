#ifndef CONFIGURACIONES_KERNEL_H
#define CONFIGURACIONES_KERNEL_H

#include <commons/config.h>
#include <global.h>
#include <configuraciones/configuraciones.h>

#define obtenerGradoMultiprogramacion() config_get_int_value   (config, "GRADO_MULTIPROGRAMACION")
#define obtenerAlgoritmoPlanificacion() config_get_string_value(config, "ALGORITMO_PLANIFICACION")
#define obtenerRecursos()               config_get_array_value (config, "RECURSOS")
#define obtenerInstanciasRecursos()     config_get_array_value (config, "INSTANCIAS_RECURSOS")
#define obtenerQuantum()                ((int64_t)config_get_int_value(config,"QUANTUM"))
typedef struct {
    char nombre_interfaz[256];
    char tipo_interfaz[256];
    int socket_interfaz;
} Interfaz;

// Estructura para gestionar el arreglo dinámico de interfaces
typedef struct {
    Interfaz *interfaces;
    size_t cantidad;
} Kernel_io;
extern t_config* config;

#endif