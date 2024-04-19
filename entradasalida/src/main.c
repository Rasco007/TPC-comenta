#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <includes/utils/utils.h>

bool sonSocketsValidos(int socketClienteKernel, int socketClienteMemoria);

int main(int argc, char* argv[]) {
   
    t_log* interfazLogger = iniciarInterfazLogger();
    if (interfazLogger == NULL) {
        return EXIT_FAILURE;        
    }

     tInterfazConfig* interfazConfig = leerInterfazConfig(interfazLogger);

    if (interfazConfig == NULL) {
        liberarLogger(interfazLogger);
        return EXIT_FAILURE;        
    }

    log_info(interfazLogger, "Hola desde Interfaz!!");


    //start cliente de kernel
    int socketClienteKernel = startCliente(interfazConfig->ip_kernel, interfazConfig->puerto_kernel, NOMBRE_CLIENTE_KERNEL, HS_IO, HS_KERNEL, interfazLogger);
    //start cliente de memoria
    int socketClienteMemoria = startCliente(interfazConfig->ip_memoria, interfazConfig->puerto_memoria, NOMBRE_CLIENTE_MEMORIA, HS_IO, HS_MEMORIA, interfazLogger);
    
    if (!sonSocketsValidos(socketClienteKernel, socketClienteMemoria)) {
        terminarPrograma(interfazConfig, interfazLogger);
        return EXIT_FAILURE;
    }

    terminarPrograma(interfazConfig, interfazLogger);

return EXIT_SUCCESS;

}

bool sonSocketsValidos(int socketClienteKernel, int socketClienteMemoria){
    return socketClienteKernel != -1 && socketClienteMemoria != -1;
}