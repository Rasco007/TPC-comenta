#include <conexionKernel/conexionKernel.h>

void conexionKernel() {
    logger = cambiarNombre (logger,"IO-Kernel");
    loggerError = cambiarNombre (loggerError, "Errores IO-Kernel");

    while(1){
        int conexionAKernel = conexion("KERNEL");
        
        if(conexionAKernel != -1){
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAKernel);
            sleep(5);
        }
    }
}

