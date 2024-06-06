#include <conexionKernel/conexionKernel.h>
#include "conexionKernel.h"

int cantidadMaximaPaginas;
uint32_t direccionBasePagina;
uint32_t tamanioPagina;
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
    cantidadMaximaPaginas = config_get_int_value(config, "TAM_PAGINA");
    while (1) {
        int peticionRealizada = recibirOperacion(*socketCliente);
        switch (peticionRealizada) {
            case NEWPCB: {
                int pid = recibirPID(*socketCliente);
                Proceso *procesoNuevo = crearProcesoEnMemoria(pid);
                // enviarTablaPaginas(procesoNuevo); // Implementar si es necesario
                break;
            }
            case ENDPCB: {
                int pid = recibirPID(*socketCliente);
                // liberarTodosLasPaginas(pid); // Implementar si es necesario
                eliminarProcesoDeMemoria(pid);
                log_info(logger, "Eliminación de Proceso PID: <%d>", pid);
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
    log_info(logger, "Creación de Proceso PID: <%d>", procesoNuevo->pid);

    return procesoNuevo;
}

void eliminarProcesoDeMemoria(int pid) {
    Proceso *proceso = buscar_proceso_por_pid(pid);
    if (proceso != NULL) {
        // Elimina todas las páginas del proceso
        liberar_tabla_paginas(proceso->tabla_paginas);
        // list_remove_element(procesos, (void *)proceso); // Implementar si es necesario
        free(proceso);
    } else {
        log_warning(logger, "Proceso PID: <%d> no encontrado para eliminación", pid);
    }
}

Proceso *buscar_proceso_por_pid(int pid) { //ver si pasar por referencia
    Proceso *proceso = NULL;
    for (int i = 0; i < NUM_MARCOS; i++) {
        if (memoria->marcos[i].pid == pid) {
            proceso = memoria->marcos[i].proceso;
        }
    }
    return proceso;
}
