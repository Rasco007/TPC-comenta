#include <conexionIO/conexionIO.h>

int ejecutarServidorIO(int * socketCliente){
    //tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");

    while (1) {
        int peticion = recibirOperacion(*socketCliente);
        log_debug(logger, "Se recibió petición %d del IO", peticion);

        switch (peticion) {
            case READ:
                recibirPeticionDeLectura(*socketCliente);
                enviarValorObtenido(*socketCliente);
                break;
            case WRITE:
                recibirPeticionDeEscritura(*socketCliente);
                enviarMensaje("OK", *socketCliente);
                break;
            case -1:
                log_error(logger, "IO se desconectó");
                return EXIT_FAILURE;
                break;
            default:
                log_warning(logger, "Operación desconocida del IO.");
                break;
        }
    }
    return EXIT_SUCCESS;
} 





