#include <conexionIO/conexionIO.h>

int ejecutarServidorIO(){
    //tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");
    
     //char *puertoEscucha = confGet("PUERTO_ESCUCHA");
    //int socketMemoria = alistarServidorMulti(puertoEscucha);

    while (1) {
        log_info(logger,"Esperando conexiones con IO...");
        //pthread_t thread;
        
        int *socketClienteIO = malloc(sizeof(int));
        
        *socketClienteIO = esperarCliente(server_fd);
        log_info(logger, "IO conectado, en socket: %d",*socketClienteIO);

        hacerHandshake(*socketClienteIO);
       

        // pthread_create(&thread,
        //                 NULL,
        //                 (void*) ejecutarServidor,
        //                 socketClienteIO);
        // pthread_detach(thread);
    }

    return EXIT_SUCCESS;
} 

void ejecutarServidor(int socketClienteIO){

    while (1) {
        int peticion = recibirOperacion(socketClienteIO);
        log_debug(logger, "Se recibió petición %d del IO", peticion);

        switch (peticion) {
            case READ:
                recibirPeticionDeLectura(socketClienteIO);
                enviarValorObtenido(socketClienteIO);
                break;
            case WRITE:
                recibirPeticionDeEscritura(socketClienteIO);
                enviarMensaje("OK", socketClienteIO);
                break;
            case -1:
                log_error(logger, "IO se desconectó");
                return EXIT_FAILURE;
                break;
            default:
                log_warning(logger, "Operación desconocida del IO. Peticion %d", peticion);
                break;
        }
    }
}



void hacerHandshake(int socketClienteIO){
    size_t bytes;

   int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(socketClienteIO, &handshake, sizeof(int32_t), MSG_WAITALL);
   
    if (handshake == 1) {
        bytes = send(socketClienteIO, &resultOk, sizeof(int32_t), 0);
    } else {
        bytes = send(socketClienteIO, &resultError, sizeof(int32_t), 0);
    }
   
}