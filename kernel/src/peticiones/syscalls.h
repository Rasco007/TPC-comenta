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
#include <main/kernel.h>
#include <commons/collections/queue.h>
extern char **nombresRecursos;
extern sem_t hayProcesosReady;
extern int *instanciasRecursos;
extern int conexionAMemoria; 
extern int tiempoIO; 
extern t_list *recursos;
extern t_contexto* contextoEjecucion;
typedef struct{
    t_pcb* proceso;
    char* tiempo;
    char* interfaz;
    t_queue* colaBloqueados;
}InterfazSalienteGenSleep;
void retornoContexto(t_pcb *, t_contexto *);
//void prc_io_gen_sleep(t_contexto *contextoEjecucion, t_pcb *proceso);
void volverACPU(t_pcb *);
void pasarAReady(t_pcb* proceso);

void recibirMsjIO(int socketClienteIO);

void wait_s(t_pcb *proceso, char **parametros);
void signal_s(t_pcb *proceso, char **parametros);
void io_stdin_read(t_pcb *proceso, char **parametros);
void io_stdout_write(t_pcb *proceso, char **parametros);
void io_fs_create(t_pcb *proceso, char **parametros);
void io_gen_sleep(t_pcb *proceso, char **parametros);
void io_fs_read(t_pcb *proceso, char **parametros);
void io_fs_write(t_pcb *proceso, char **parametros);
void io_fs_delete(t_pcb *proceso, char **parametros);
void io_fs_truncate(t_pcb *proceso, char **parametros);
void exit_s(t_pcb *proceso, char **parametros);
void finDeQuantum(t_pcb *proceso);

void loggearBloqueoDeProcesos(t_pcb*,char* motivo); 
void loggearSalidaDeProceso(t_pcb*,char* motivo);

//void *mandar_ejecutar_stdout(t_pcb *proceso, char *interfaz,char *registroDireccion, char* registroTamanio);
//void *mandar_ejecutar_stdin(char *interfaz,char *registroDireccion, char* registroTamanio);
void enviarMensajeGen(int socket_cliente, char *mensaje, char *entero_str, int pid);
//void enviarMensajeSTDIN(int socketClienteIO, char* nombreInterfaz, char* registroDireccion, char *registroTamanio);
//void enviarMensajeSTDOUT(int socketClienteIO, char* nombreInterfaz, char* registroDireccion, char *registroTamanio);

#endif /* SYSCALLS_H_ */