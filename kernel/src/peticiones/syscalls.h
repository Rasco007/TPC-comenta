#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <pthread.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <utilsServidor/utilsServidor.h>
#include <conexiones/conexionMemoria.h>
#include <escuchaIO/servidorIO.h>
#include <peticiones/pcb.h>
#include <planificacion/planificacion.h>
#include <planificacion/algoritmosCortoPlazo.h>
#include <peticiones/manejoRecursos.h>
#include <peticiones/manejoPaginas.h>

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

void* dormirIO(t_pcb * proceso, char* interfaz,char* tiempo);

void wait_s(t_pcb *proceso, char **parametros);
void resize_s(t_pcb *proceso, char **parametros);
void signal_s(t_pcb *proceso, char **parametros);
void io_gen_sleep(t_pcb *proceso, char **parametros);
void io_stdin_read(t_pcb *proceso, char **parametros);
void io_stdout_write(t_pcb *proceso, char **parametros);
void io_fs_create(t_pcb *proceso, char **parametros);
void io_fs_read(t_pcb *proceso, char **parametros);
void io_fs_write(t_pcb *proceso, char **parametros);
void io_fs_delete(t_pcb *proceso, char **parametros);
void io_fs_truncate(t_pcb *proceso, char **parametros);
void exit_s(t_pcb *proceso, char **parametros);


void loggearBloqueoDeProcesos(t_pcb*,char* motivo); 

void loggearSalidaDeProceso(t_pcb*,char* motivo); 

#endif /* SYSCALLS_H_ */