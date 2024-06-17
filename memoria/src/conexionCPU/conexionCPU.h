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

// Definiciones de las operaciones
#define READ 0
#define WRITE 1

// Variables globales
extern int tiempo;
extern MemoriaFisica *memoria;

// Funciones
int ejecutarServidorCPU(int *socketCliente);
char* leer(int32_t direccionFisica, int tamanio);
void recibirPeticionDeLectura(int socketCPU);
void recibirPeticionDeEscritura(int socketCPU);
void enviarValorObtenido(int socketCPU);
void escribir(char* valor, int32_t direccionFisica, int tamanio);

Proceso *ajustar_tamano_proceso(MemoriaFisica *mf,Proceso *proceso, int nuevo_tamano);

#endif // MEMORIA_CONEXION_CPU_H
