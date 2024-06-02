#ifndef ESTRUCTURA_H
#define ESTRUCTURA_H

#include <stdlib.h>
#include <stdint.h>

// Estructura para las entradas de la tabla de páginas
typedef struct {
    int numeroDePagina;
    int numeroDeMarco;
    int bitDeValidez;
} EntradaTablaDePaginas;

// Estructura para la tabla de páginas
typedef struct {
    int numeroDeEntradas;
    EntradaTablaDePaginas* entradas;
} TablaDePaginas;

// Estructura para la memoria
typedef struct {
    void* memoria;             // Espacio de memoria contiguo
    TablaDePaginas* tabla_paginas;  // Tabla de páginas
} Memoria;

typedef struct {
    uint32_t pid; 
    TablaDePaginas* TablaDePaginas;
} t_proceso; 

typedef struct {
    uint32_t pid; 
    EntradaTablaDePaginas* pagina; 
} t_peticion; 


// Función para crear la memoria
Memoria* crearMemoria(int num_paginas, int tamano_pagina);

// Función para destruir la memoria
void destruirMemoria(Memoria* memoria);

// Función para escribir en memoria
void escribirMemoria(Memoria* memoria, int direccion_logica, void* datos, int tamano);

// Función para leer de memoria
void* leerMemoria(Memoria* memoria, int direccion_logica, int tamano);

// Función para inicializar la tabla de páginas
TablaDePaginas* inicializarTablaDePaginas(int numeroDeEntradas);

// Función para liberar la memoria utilizada por la tabla de páginas
void liberarTablaDePaginas(TablaDePaginas* tabla);

#endif // ESTRUCTURA_H
