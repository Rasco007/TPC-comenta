#include <conexionKernel/conexionKernel.h>
#include "conexionKernel.h"

int cantidadMaximaPaginas;
uint32_t direccionBasePagina;
uint32_t tamanioPagina;

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

/*Proceso *buscar_proceso_por_pid(int pid) {
    Proceso *proceso = NULL;
    for (int i = 0; i < list_size(procesos); i++) {
        Proceso *p = list_get(procesos, i);
        if (p->pid == pid) {
            proceso = p;
            break;
        }
    }
    return proceso;
}*/
