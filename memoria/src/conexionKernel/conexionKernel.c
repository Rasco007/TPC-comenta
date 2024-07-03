#include <conexionKernel/conexionKernel.h>
#include "conexionKernel.h"


int cantidadMaximaPaginas;
uint32_t direccionBasePagina;
uint32_t tamanioPagina;
char* pathInstrucciones;
sem_t path;



uint32_t recibirPID(int socketCliente) {

	int size, desplazamiento=0; 
	uint32_t pid; 

	void* buffer = recibirBuffer(socketCliente, &size);
	desplazamiento += sizeof(int);
	memcpy(&(pid), buffer + desplazamiento, sizeof(uint32_t));
    memcpy(pathInstrucciones,buffer+sizeof(int),sizeof(pathInstrucciones));
	free (buffer);
	return pid; 

}
int ejecutarServidorKernel(int *socketCliente) {
    cantidadMaximaPaginas = confGetInt("TAM_PAGINA");
    while (1) {
        int peticionRealizada = recibirOperacion(*socketCliente);
        switch (peticionRealizada) {
            case NEWPCB: {
                int pid = recibirPID(*socketCliente);
                Proceso *procesoNuevo = crearProcesoEnMemoria(pid,pathInstrucciones);
                //enviarTablaPaginas(procesoNuevo);
                log_info(logger,"Creacion de Proceso PID: <%d>", pid);
                break;
            }
            case ENDPCB: {
                int pid = recibirPID(*socketCliente);
                // liberarTodosLasPaginas(pid); // Implementar si es necesario
                eliminarProcesoDeMemoria(pid);
                log_info(logger, "Eliminación de Proceso PID: <%d>", pid);
                break;
            }
            case MENSAJE:{
                pathInstrucciones=recibirMensaje(*socketCliente); //Recibo el path
                log_info(logger,"Path de instrucciones recibido: %s",pathInstrucciones);
                sem_post(&path);
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

Proceso *crearProcesoEnMemoria(int pid, char* pathInstrucciones) {
    Proceso *procesoNuevo = malloc(sizeof(Proceso));
    procesoNuevo->pid = pid;
    procesoNuevo->tabla_paginas = inicializar_tabla_paginas();
    // list_add(procesos, (void *)procesoNuevo); // Implementar si es necesario
    FILE *archivo = fopen(pathInstrucciones, "r");
    if (!archivo) {
        perror("Error al abrir el archivo de pseudocódigo");
        free(procesoNuevo);
        return NULL;
    }
    
    procesoNuevo->numero_instrucciones = 0;
    procesoNuevo->instrucciones = NULL;
    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        procesoNuevo->numero_instrucciones++;
        procesoNuevo->instrucciones = realloc(procesoNuevo->instrucciones, procesoNuevo->numero_instrucciones * sizeof(char *));
        procesoNuevo->instrucciones[procesoNuevo->numero_instrucciones - 1] = string_duplicate(linea);
    }
    fclose(archivo);
    //printf("tamaño del proceso %lu\n", sizeof(proceso->tabla_paginas));
    //Guardo el numero de instrucciones del proceso como un char* para mandarlo a kernel
    sprintf(numeroDeInstrucciones, "%d", procesoNuevo->numero_instrucciones); 
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
    log_info(logger, "buscar_proceso_por_pid: %d",pid);
    Proceso *proceso = NULL;
    for (int i = 0; i < NUM_MARCOS; i++) {
        if (mf->marcos[i].pid == pid) {
            proceso = mf->marcos[i].proceso;
        }
    }
    return proceso;
}
