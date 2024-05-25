#include <conexionKernel/conexionKernel.h>

void conexionKernel() {
   
    while(1){
        int conexionAKernel = conexion("KERNEL");
        
        if(conexionAKernel != -1){
            break;
        }
        else {
            log_error(io_loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAKernel);
            sleep(5);
        }
    }
}

