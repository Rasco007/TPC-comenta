#ifndef PSEUDOCODIGO_H
#define PSEUDOCODIGO_H

#include <stdio.h>
#include <stdlib.h>
#include <estructura/estructura.h>

typedef struct {
    char** instrucciones;
    int cantidad_instrucciones;
} Pseudocodigo;

Pseudocodigo* leerPseudocodigo(const char* ruta);
void liberarPseudocodigo(Pseudocodigo* pseudo);
char* obtenerInstruccion(Pseudocodigo* pseudo, int indice);
void crear_proceso(Memoria* memoria, int pid, int tamanio, const char* ruta_pseudocodigo);
void destruir_proceso(Memoria* memoria, int pid, int tamanio);

#endif // PSEUDOCODIGO_H
