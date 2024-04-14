#include <stdlib.h>
#include <stdio.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <client/conexionMemoria.h>
#include <server/escucharKernel.h>
//programa principal-cpu

int main(void){
	t_log* cpuLogger=iniciarCPULogger();
	tCPUconfig* cpuConfig=leerConfig(cpuLogger); 
	int conexion=conexionMemoria(cpuLogger);
	//escucharkernel()
	terminar_programa(conexion,cpuLogger,cpuConfig);
	return 0;
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}