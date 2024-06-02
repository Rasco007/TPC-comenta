#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <pthread.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsServidor/utilsServidor.h>
#include <conexiones/conexionMemoria.h>
#include <peticiones/pcb.h>
#include <planificacion/planificacion.h>
#include <planificacion/algoritmosCortoPlazo.h>


extern char **nombresRecursos;
extern sem_t hayProcesosReady;
extern int *instanciasRecursos;
extern int conexionAMemoria; 
extern int tiempoIO; 
extern t_list *recursos;
extern t_list* pcbsEnMemoria;
extern t_contexto* contextoEjecucion;

void retornoContexto(t_pcb*, t_contexto*);
void volverACPU(t_pcb*); 

void bloquearIO(t_pcb * proceso);

void wait_s(t_pcb *proceso, char **parametros);

void signal_s(t_pcb *proceso, char **parametros);

void loggearBloqueoDeProcesos(t_pcb*,char* motivo); 

void loggearSalidaDeProceso(t_pcb*,char* motivo); 

#endif /* SYSCALLS_H_ */