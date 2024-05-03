/* KERNEL- cliente | MEMORIA - servidor*/
#include <conexiones/conexionMemoria.h>

int conexionAMemoria;

void conexionMemoria() {
    logger = cambiarNombre (logger,"Kernel-Memoria");
    loggerError = cambiarNombre (loggerError, "Errores Kernel-Memoria");

    while(1){
        conexionAMemoria = conexion("MEMORIA");
        
        if(conexionAMemoria != -1){
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAMemoria);
            sleep(5);
        }
    }
}

void recibirEstructurasInicialesMemoria(t_pcb* pcb) {
    
    char * nombreAnterior = duplicarNombre(logger);
    logger = cambiarNombre(logger,"Kernel-Memoria");
    
    t_paquete* peticion = crearPaquete(); 
    peticion->codigo_operacion = NEWPCB; 
    agregarAPaquete(peticion,(void*)&pcb->pid, sizeof(uint32_t));
    enviarPaquete(peticion, conexionAMemoria); 
    eliminarPaquete (peticion);
    debug ("PID <%d>: Se esta solicitando estructuras iniciales de memoria.", pcb->pid);
    recibirOperacion (conexionAMemoria);
    logger = cambiarNombre(logger, nombreAnterior);
    free (nombreAnterior);
}
