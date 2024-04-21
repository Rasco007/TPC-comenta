#include <conexiones/conexionIO.h>

int conexionAIO;

void conexionIO() {
    logger = cambiarNombre (logger,"Kernel-IO");
    loggerError = cambiarNombre (loggerError, "Errores Kernel-IO");

    while(1){
        conexionAIO = conexion("IO");
        
        if(conexionAIO == -1){
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionAIO);
            sleep(5);
        }

        return;
    }
}

