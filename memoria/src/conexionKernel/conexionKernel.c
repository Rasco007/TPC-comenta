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
                mf->marcos[PID].proceso=proceso;
                mf->marcos[PID].pid=PID;

                //Mando el numero de instrucciones a kernel
                int n=mf->marcos[PID].proceso->numero_instrucciones;
                log_info(logger,"Cantidad de instrucciones: %d",n);
                mandarNumInstrucciones(n,*socketCliente);
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

Proceso *crearProcesoEnMemoria(int pid) {
    Proceso *procesoNuevo = malloc(sizeof(Proceso));
    procesoNuevo->pid = pid;
    procesoNuevo->tabla_paginas = inicializar_tabla_paginas();
    // list_add(procesos, (void *)procesoNuevo); // Implementar si es necesario

    return procesoNuevo;
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
    for (int i = 0; i < NUM_MARCOS; i++) {
        if (mf->marcos[i].pid == pid) {
            proceso = mf->marcos[i].proceso;
        }
    }
    return proceso;
}

void mandarNumInstrucciones(int numero, int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = 2*sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	
	memcpy(paquete->buffer->stream, &numero, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
	
    void *a_enviar = serializarPaquete(paquete, bytes);

    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar dato");
        exit(EXIT_FAILURE);
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
	free(a_enviar);
	free(paquete);
}