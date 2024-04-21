#ifndef MEMORIA_CONEXION_KERNEL_H
#define MEMORIA_CONEXION_KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>

extern t_config* config;
extern t_log* logger;
extern void* espacioDeUsuario;
extern t_list* huecosLibres; 
extern t_list* tablaDeTablasDeSegmentos; 
extern int sockets[2];

int ejecutarServidorKernel(int*);
t_list* crearTablaDeSegmentosInicial(uint32_t); 

uint32_t recibirPID(int socketCliente); 

void recibirYProcesarPeticionEliminacionSegmento(int socketCliente); 

void procesarResultado(int resultado, int socketKernel); 

void deleteSegment(uint32_t pid, uint32_t segmentId);

void convertirSegmentoEnHuecoLibre(void* segmentoAEliminar);

void listarHuecosLibres ();

void liberarTodosLosSegmentos(uint32_t pid);

bool hayHuecoLibreArriba(void* huecoLibre);

bool hayHuecoLibreAbajo(void* huecoLibre);


#endif