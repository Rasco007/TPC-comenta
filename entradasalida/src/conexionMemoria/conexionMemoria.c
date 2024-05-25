#include <conexionMemoria/conexionMemoria.h>

void conexionMemoria() {
   
    while(1){
        int conexionAMemoria = conexion("MEMORIA");
        
        if(conexionAMemoria != -1){
            break;
        }
        else {
            log_error(io_loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAMemoria);
            sleep(5);
        }
    }
}
