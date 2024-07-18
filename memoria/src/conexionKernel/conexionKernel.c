#include <conexionKernel/conexionKernel.h>
#include "conexionKernel.h"

int PID;
int cantidadMaximaPaginas;
uint32_t direccionBasePagina;
uint32_t tamanioPagina;
char* pathInstrucciones;
sem_t path;
//serializar tabla de Segmentos


uint32_t recibirPID(int socketCliente) {

	int size, desplazamiento=0; 
	uint32_t pid; 

	void* buffer = recibirBuffer(socketCliente, &size);
	desplazamiento += sizeof(int);
	memcpy(&(pid), buffer + desplazamiento, sizeof(uint32_t));

	free (buffer);
	return pid; 

}
int ejecutarServidorKernel(int *socketCliente) {
    cantidadMaximaPaginas = confGetInt("TAM_PAGINA");
    while (1) {
        int peticionRealizada = recibirOperacion(*socketCliente);
        log_info(logger, "entro while");
        switch (peticionRealizada) {
            case NEWPCB: {
                PID = recibirPID(*socketCliente);
                //enviarTablaPaginas(procesoNuevo);
                
                Proceso *proceso = inicializar_proceso(PID, pathInstrucciones);
                
                int n =proceso->numero_instrucciones;
                //Mando el numero de instrucciones a kernel
                log_info(logger,"Cantidad de instrucciones: %d",n);
                send(*socketCliente, &n, sizeof(int), 0);
                log_info(logger,"Creacion de Proceso PID: <%d>", PID);
                break;
            }
            case ENDPCB: {
                PID = recibirPID(*socketCliente);
                // liberarTodosLasPaginas(pid); // Implementar si es necesario
                eliminarProcesoDeMemoria(PID);
                log_info(logger, "Eliminación de Proceso PID: <%d>", PID);
                break;
            }
            case MENSAJE:{
                pathInstrucciones=recibirMensaje(*socketCliente); //Recibo el path
                log_info(logger,"Path de instrucciones recibido: %s",pathInstrucciones);
                //sem_post(&path);
                break;
            }
            case -1:
                log_error(logger, "El Kernel se desconectó");
                return EXIT_FAILURE;
            default:
                log_warning(logger, "Operación desconocida: %d", peticionRealizada);
                break;
        }
    }
    return EXIT_SUCCESS;
}



void eliminarProcesoDeMemoria(int pid) {
   Proceso *proceso = buscar_proceso_por_pid(pid);
    if (proceso != NULL) {
        // Elimina todas las páginas del proceso
        liberar_tabla_paginas(proceso->tabla_paginas);

        // Elimina todas las instrucciones del proceso
        for (int i = 0; i < proceso->numero_instrucciones; i++) {
            free(proceso->instrucciones[i]);
        }
        free(proceso->instrucciones);

        // Elimina el proceso de la lista (si es necesario)
        // list_remove_element(procesos, (void *)proceso); // Implementar si es necesario

        //bbusco procesoo en la lisssstaa y lo eliminoo
        for (int i = 0; i < list_size(mf->listaProcesos); i++) {
        proceso = list_get(mf->listaProcesos,i);
        if (proceso->pid == pid) {
            
            list_remove(mf->listaProcesos,i);

        }
    }

        // Libera la memoria del proceso
        free(proceso);

        log_info(logger, "Proceso PID: <%d> eliminado correctamente", pid);
    } else {
        log_warning(logger, "Proceso PID: <%d> no encontrado para eliminación", pid);
    }
}

Proceso *buscar_proceso_por_pid(int pid) { //ver si pasar por referencia
    log_info(logger, "buscar_proceso_por_pid: %d",pid);
    Proceso *proceso = NULL;
    for (int i = 0; i < list_size(mf->listaProcesos); i++) {
        proceso = list_get(mf->listaProcesos,i);
        if (proceso->pid == pid) {
            
            return proceso;
        }
    }
    return NULL;
}