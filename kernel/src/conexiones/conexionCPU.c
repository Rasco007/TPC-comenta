/* KERNEL- cliente | CPU - servidor*/
#include <conexiones/conexionCPU.h>


t_buffer* bufferContexto;
int conexionACPU;
int conexionACPUInterrupt;

void conexionCPU() {

    //CONEXION CPU DISPATCH
    logger = cambiarNombre(logger, "Kernel-CPU (dispatch)");
    loggerError = cambiarNombre(loggerError,"Errores Kernel-CPU (dispatch)");

    while(1){
        conexionACPU = conexion("CPU_DISPATCH");
        
        if(conexionACPU != -1){
           break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionACPU);
            sleep(5);
        }
    }

    //CONEXION CPU INTERRUPT
    logger = cambiarNombre(logger, "Kernel-CPU (interrupt)");
    loggerError = cambiarNombre(loggerError,"Errores Kernel-CPU (interrupt)");

    while(1){
        conexionACPUInterrupt = conexion("CPU_INTERRUPT");
        
        if(conexionACPUInterrupt != -1){
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionACPUInterrupt);
            sleep(5);
        }
    }
}

int recibirOperacionDeCPU(){ 
	int cod_op;
    
	if (recv(conexionACPU, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(conexionACPU);
		return -1;
	}
}

t_contexto* procesarPCB(t_pcb* procesoEnEjecucion) {
    /*if (contextoEjecucion != NULL) destroyContextoUnico ();
	iniciarContexto ();

    bufferContexto = malloc(sizeof(t_buffer));

    asignarPCBAContexto(procesoEnEjecucion);
    
    //t_segmento * test = (t_segmento *) list_get (procesoEnEjecucion->tablaDeSegmentos, 0);

    //debug ("Test: %d %d %d", test->direccionBase, test->id, test->tamanio);

    enviarContextoActualizado(conexionACPU);

    if (recibirOperacionDeCPU() < 0) error ("Se desconecto el CPU.");

    recibirContextoActualizado(conexionACPU); 

    actualizarPCB(procesoEnEjecucion);

    //uint32_t lista = list_size (procesoEnEjecucion->recursosAsignados);
    //debug ("Tengo %d recursos.", lista);
    //for (uint32_t i = 0; i < lista; i++) 
    //    debug ("Listando recursos, recurso %d: %s", i, (char *) list_get (procesoEnEjecucion->recursosAsignados, i));

    free(bufferContexto);
    return contextoEjecucion; TODO! */
 
}