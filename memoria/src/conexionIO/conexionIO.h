#ifndef MEMORIA_CONEXION_IO_H
#define MEMORIA_CONEXION_IO_H

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <global.h>
#include <./main/memoria.h>
#include <pthread.h>
#include <string.h>
int ejecutarServidorIO(); 
void* ejecutarServidor(void* socketCliente) ;
int ejecutarServidorCPU(int *socketCliente);
char* leer(int32_t direccionFisica, int tamanio);
void recibirPeticionDeLectura(int socketCPU);
void recibirPeticionDeEscritura(int socketCPU);
void enviarValorObtenido(int socketCPU);
void escribir(char* valor, int32_t direccionFisica, int tamanio);
void hacerHandshake2(int socketClienteIO);
void recibirDirYCadena(int socket, int *dir, char* cadena);
void recibirDireccionyTamano(int socket, int *dir, int *tamano);
#endif 



