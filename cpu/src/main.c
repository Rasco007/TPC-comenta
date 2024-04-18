#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <client_handler/client_handler.h>
#include <stdlib.h>

bool sonSocketsValidos(int socketServidorKernelDispatch, int socketServidorKernelInterrupt, int socketClienteMemoria);

int main(int argc, char* argv[]) {
    t_log* cpuLogger=iniciarCPULogger();
    if(cpuLogger==NULL){
        return EXIT_FAILURE;}

	tCPUconfig* cpuConfig=leerCPUConfig(cpuLogger); 
    if(cpuConfig==NULL){
        log_destroy(cpuLogger);
        return EXIT_FAILURE;
    }

    log_info(cpuLogger,"Hola desde la CPU!!");

    //conexion a memoria
    int socketClienteMemoria=startCliente(cpuConfig->ipMemoria,
    cpuConfig->puertoMemoria,
    NOMBRE_CLIENTE_MEMORIA,
    HS_CPU_DISPATCH, //???
    HS_MEMORIA,cpuLogger);
    
    //Escuchar Kernel
    char* ip; //De donde sacaria esto? 
    int socketServidorKernelDispatch=iniciarServidor(cpuLogger,ip,cpuConfig->puertoEscuchaDispatch);
    int socketServidorKernelInterrupt=iniciarServidor(cpuLogger,ip,cpuConfig->puertoEscuchaInterrupt);

    if (!sonSocketsValidos(socketServidorKernelDispatch, socketServidorKernelInterrupt, socketClienteMemoria)) {
        //Terminar programa
        log_destroy(cpuLogger);
        config_destroy(cpuConfig);
        liberarConexion(socketClienteMemoria);
        liberarConexion(socketServidorKernelDispatch);
        liberarConexion(socketServidorKernelInterrupt);
        return EXIT_FAILURE;
    }

    //Terminar programa
    log_destroy(cpuLogger);
    config_destroy(cpuConfig);
    liberarConexion(socketClienteMemoria);
    liberarConexion(socketServidorKernelDispatch);
    liberarConexion(socketServidorKernelInterrupt);
    
    return EXIT_SUCCESS;
}

bool sonSocketsValidos(int socketServidorKernelDispatch, int socketServidorKernelInterrupt, int socketClienteMemoria) {
    return socketServidorKernelDispatch != -1 && socketServidorKernelInterrupt != -1 && socketClienteMemoria != -1;
}


