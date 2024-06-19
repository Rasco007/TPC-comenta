#ifndef ESTRUCTURA_H
#define ESTRUCTURA_H

#include <stdlib.h>
#include <stdint.h>

#include <commons/string.h>

// Definiciones para la memoria física
#define TAMANO_PAGINA 4096 // Tamaño de cada página (4KB)
#define NUM_MARCOS 64      // Número total de marcos en la memoria física

// Definiciones para la tabla de páginas
#define NUM_PAGINAS 256 // Número total de páginas en la memoria
typedef struct {
    int valido;        // Indica si la entrada de la tabla de páginas es válida
    int numero_marco;   // Número de marco en la memoria física
} EntradaTablaPaginas;

typedef struct {
    EntradaTablaPaginas entradas[NUM_PAGINAS];
} TablaPaginas;

typedef struct {
    int pid;
    TablaPaginas *tabla_paginas;
    char **instrucciones;
    int numero_instrucciones;
} Proceso;
typedef struct {
    int libre;         // Indica si el marco está libre
    int numero_pagina;  // Página asignada a este marco
    int pid;     // Proceso al que pertenece este marco
    Proceso* proceso;
} Marco;
typedef struct {
    void *memoria;      // Memoria física simulada
    Marco marcos[NUM_MARCOS]; // Arreglo de marcos
} MemoriaFisica;

MemoriaFisica *inicializar_memoria_fisica();
void liberar_memoria_fisica(MemoriaFisica *mf);

TablaPaginas *inicializar_tabla_paginas();
void liberar_tabla_paginas(TablaPaginas *tp);

// Definiciones para el proceso
Proceso *inicializar_proceso(int pid, const char *archivo_pseudocodigo);
void liberar_proceso(Proceso *proceso);
char *obtener_instruccion(Proceso *proceso, int program_counter);

bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina);

#endif // ESTRUCTURA_H
