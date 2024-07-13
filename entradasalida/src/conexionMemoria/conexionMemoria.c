#include <conexionMemoria/conexionMemoria.h>

void conexionMemoria(char **argv) {
char *nombreInterfaz = argv[1];
    while(1){
        fd_memoria = conexion("MEMORIA");
        
        if(fd_memoria != -1){
            enviarHandshake(nombreInterfaz);
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", fd_memoria);
            sleep(5);
        }
    }
}
void enviarHandshake(char *nombreInterfaz)
{
    size_t bytes;

    int32_t handshake = 1;
    int32_t result;

    bytes = send(fd_memoria, &handshake, sizeof(int32_t), 0);
    bytes = recv(fd_memoria, &result, sizeof(int32_t), MSG_WAITALL);

    if (result == 0)
    {
        log_info(logger, "Handshake OK");
       
    }
    else
    {
        log_error(logger, "Handshake ERROR");
    }
}