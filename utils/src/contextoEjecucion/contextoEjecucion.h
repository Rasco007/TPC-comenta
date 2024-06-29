#ifndef CONTEXTO_EJECUCION_H
#define CONTEXTO_EJECUCION_H

#include <global.h>
#include <utilsCliente/utilsCliente.h>
#include <utilsServidor/utilsServidor.h>
#include <commons/temporal.h>

#include "stdlib.h"
#include "stdint.h"
#include "math.h"

typedef struct {

        t_comando motivo; 
        int parametrosLength;
        char* parametros[3]; 

}t_motivoDeDesalojo; 

typedef struct {
    int idPagina;
    int idFrame;
    int bitDeValidez;
}t_pagina; //Ver si es correcto

typedef enum{
    FIFO,
    RR,
    VRR,
} t_algoritmo;

typedef struct {
    uint32_t pid; 
    int programCounter;
    uint32_t SI;
    uint32_t DI;
    uint32_t instruccionesLength;
    t_list* instrucciones; 
    t_dictionary* registrosCPU;  
    uint32_t tablaDePaginasSize;
    t_list* tablaDePaginas; 
    t_motivoDeDesalojo* motivoDesalojo;
    t_temporal* tiempoDeUsoCPU;
    int64_t quantum;
    t_algoritmo algoritmo;
} t_contexto;

extern t_contexto* contextoEjecucion;
extern int socketCliente;


void agregarMotivoAPaquete(t_paquete* paquete, t_motivoDeDesalojo* motivoDesalojo);
void agregarRegistrosAPaquete(t_paquete* paquete, t_dictionary* registrosCPU);
void agregarInstruccionesAPaquete(t_paquete* paquete, t_list* instrucciones);
void agregarTablaDePaginasAPaquete(t_paquete* paquete);
void agregarPaginaAPaquete(t_paquete* paquete, t_pagina* pagina);

// FUNCIONES PARA ENVIO DE CONTEXTO DE EJECUCION
void enviarContextoActualizadoUtils(int socket,t_contexto * contextoEjecucion );
void enviarContextoActualizado(int socket);

//FUNCIONES PARA RECIBIR CONTEXTO DE EJECUCION
void recibirContextoActualizado(int socket, t_contexto *contextoEjecucion); 
void deserializarInstrucciones (void * buffer, int * desplazamiento);
void deserializarRegistros (void * buffer, int * desplazamiento);
void deserializarMotivoDesalojo (void * buffer, int * desplazamiento);
t_pagina*  deserializarPagina(void* buffer, int* desplazamiento);
void deserializarTablaDePaginas (void * buffer, int * desplazamiento); 

void iniciarContexto();
void destroyContexto();
void destroyContextoUnico();



#endif