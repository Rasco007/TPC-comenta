#ifndef MANEJO_SEGMENTOS_H_
#define MANEJO_SEGMENTOS_H_

#include <commons/collections/list.h>
#include <conexiones/conexionMemoria.h>
#include <peticiones/pcb.h>
#include <contextoEjecucion/contextoEjecucion.h>

extern int conexionAMemoria;

void recibirTablaDePaginasActualizada(t_pcb* pcb);
uint32_t recibirPID(int socketCliente) ;

#endif /* MANEJO_SEGMENTOS_H_ */