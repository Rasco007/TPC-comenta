#include <client/conexionMemoria.h>

int conexionMemoria;
int conexionMemoria(t_log* logger){
    while(1){
        //conexionMemoria=crearConexion() 

        if(conexionMemoria!=1){
            return 0;
        }
        else{
            log_error(logger,
            "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.",
            conexionMemoria);
            sleep(5);
        }
    }
}
