#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <client_handler/client_handler.h>
#include <stdlib.h>

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
    
    //EscucharKernel()

    terminar_programa(socketClienteMemoria,cpuLogger,cpuConfig);
    return 0;
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}