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
#include <conexionKernel/conexionKernel.h>

// Variables globales
extern int tiempo;
extern MemoriaFisica *mf;
extern int indice;

// Funciones
int ejecutarServidorCPU(int *socketCliente);
void enviarValorObtenido(int socketCPU);

Proceso *ajustar_tamano_proceso(MemoriaFisica *mf,Proceso *proceso, int nuevo_tamano);

void BuscarYEnviarMarco (int pid, int pagina,char *marco,int socketCliente);
void recibirEnteros(int socket, int *pid, int *pagina);
#endif // MEMORIA_CONEXION_CPU_H