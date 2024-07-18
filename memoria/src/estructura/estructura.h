#ifndef ESTRUCTURA_H
#define ESTRUCTURA_H

#include <stdlib.h>
#include <stdint.h>

#include <commons/string.h>
#include <commons/bitarray.h>
#include <configuraciones/configuraciones.h>

// Definiciones para la memoria física
#define TAM_MEMORIA confGetInt("TAM_MEMORIA")
#define TAM_PAGINA confGetInt("TAM_PAGINA")
#define CANT_PAGINAS TAM_MEMORIA / TAM_PAGINA

typedef struct {
    int valido;        
    int numero_marco;  
    int numero_pagina;
} EntradaTablaPaginas;


typedef struct {
    t_list* entradas;
    int paginas_asignadas; 
} TablaPaginas;

typedef struct {
    int pid;
    TablaPaginas* tabla_paginas;
    char **instrucciones;
    int numero_instrucciones;
} Proceso;

typedef struct {
    void* memoria;
    t_bitarray* tablaMarcosLibres;
} MemoriaFisica;

MemoriaFisica *inicializar_memoria_fisica(int tamano_pagina);
void liberar_memoria_fisica(MemoriaFisica *mf);

TablaPaginas *inicializar_tabla_paginas();
void liberar_tabla_paginas(TablaPaginas *tp);

// Definiciones para el proceso
Proceso *inicializar_proceso(int pid, const char *archivo_pseudocodigo);
void liberar_proceso(Proceso *proceso);
char *obtener_instruccion(Proceso *proceso, int program_counter);

bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina);

#endif // ESTRUCTURA_H
