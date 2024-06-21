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

// Variables globales
extern int tiempo;
extern MemoriaFisica *memoria;
extern int indice;

// Funciones
int ejecutarServidorCPU(int *socketCliente);
char* leer(int32_t direccionFisica, int tamanio);
void recibirPeticionDeLectura(int socketCPU);
void recibirPeticionDeEscritura(int socketCPU);
void enviarValorObtenido(int socketCPU);
void escribir(char* valor, int32_t direccionFisica, int tamanio);

Proceso *ajustar_tamano_proceso(MemoriaFisica *mf,Proceso *proceso, int nuevo_tamano);

void BuscarYEnviarMarco (int pid, int pagina,char *marco,int socketCliente);
void recibirEnteros(int socket, int *pid, int *pagina);
#endif // MEMORIA_CONEXION_CPU_H