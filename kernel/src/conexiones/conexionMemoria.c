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

//PETICIONES KERNEL-MEMORIA

void recibirEstructurasInicialesMemoria(t_pcb* pcb) {
    char * nombreAnterior = duplicarNombre(logger);
    logger = cambiarNombre(logger,"Kernel-Memoria");
    
    t_paquete* peticion = crearPaquete(); 
    peticion->codigo_operacion = NEWPCB; 

    agregarAPaquete(peticion,(void*)&pcb->pid, sizeof(uint32_t));
    enviarPaquete(peticion, conexionAMemoria); 
    eliminarPaquete (peticion);

    log_info(logger,"PID <%d>: Se esta solicitando estructuras iniciales de memoria.", pcb->pid);
    //recibirOperacion (conexionAMemoria);
    logger = cambiarNombre(logger, nombreAnterior);
    free (nombreAnterior);
}

void enviarPathDeInstrucciones(char* path){
    char * nombreAnterior = duplicarNombre(logger);
    logger = cambiarNombre(logger,"Kernel-Memoria");

    enviarMensaje(path,conexionAMemoria);
    
    log_info(logger,"Se envio el path de instrucciones a memoria.");
    logger = cambiarNombre(logger, nombreAnterior);
    free (nombreAnterior);
}


void liberarMemoriaPCB(t_pcb* proceso){
    
    char * nombreAnterior = duplicarNombre(logger);
    logger = cambiarNombre(logger,"Kernel-Memoria");

    log_info(logger, "PID <%d>: Se envia señal para eliminar estructuras en memoria.", proceso->pid);
    logger = cambiarNombre(logger, nombreAnterior);
    free (nombreAnterior);

    t_paquete* peticion = crearPaquete(); 
    peticion->codigo_operacion = ENDPCB; 
    agregarAPaquete(peticion,(void*)&proceso->pid, sizeof(uint32_t));
    enviarPaquete(peticion, conexionAMemoria); 
    eliminarPaquete (peticion);
}