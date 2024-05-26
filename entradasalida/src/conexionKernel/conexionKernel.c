#include <conexionKernel/conexionKernel.h>

void conexionKernel() {
   
    while(1){
         fd_kernel = conexion("KERNEL");
        
        if(fd_kernel != -1){
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", fd_kernel);
            sleep(5);
        }
    }
}

