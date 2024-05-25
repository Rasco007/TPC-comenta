#include <conexionMemoria/conexionMemoria.h>

void conexionMemoria() {
                                   log_error(io_loggerError, "antes de while");

    while(1){
        int conexionAMemoria = conexion("MEMORIA");
                                   log_error(io_loggerError, "socket %d", conexionAMemoria);

        if(conexionAMemoria != -1){
                                   log_error(io_loggerError, "entra aca");

            break;
        }
        else {
                                               log_error(io_loggerError, "entra aca en pos");

            log_error(io_loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAMemoria);
            sleep(5);
        }
    }
}
