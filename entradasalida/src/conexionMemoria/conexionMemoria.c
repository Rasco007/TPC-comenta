#include <conexionMemoria/conexionMemoria.h>

void conexionMemoria() {

    while(1){
         fd_memoria = conexion("MEMORIA");

        if(fd_memoria != -1){

            break;
        }
        else {

            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", fd_memoria);
            sleep(5);
        }
    }
}
