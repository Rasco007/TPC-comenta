#include "cpu/include/cpu.h"
//programa principal-cpu


int main(void){
	//inicializo los loggers
	t_log* nuevoLogger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);
	if(nuevoLogger == NULL) {
		perror("No se pudo crear el logger");
		exit(1);
	}

	t_log* errorLogger = log_create("errores.log", "Errores CPU", true, LOG_LEVEL_INFO);
	if(errorLogger == NULL) {
		perror("No se pudo crear el logger");
		exit(1);
	}

    //inicializo el config
	t_config* nuevoConfig = config_create("cpu.config"); 
    if(nuevoConfig==NULL) {
		log_error(logger,"Error al generar archivo de config"); 
		log_destroy(logger); 
		exit(1); 
	}

	
    
	conexionMemoria(); //Serian dos conexiones??
    
	//Comienzo escucha al kernel
	t_log *nuevoLogger = logger;
	free(logger->program_name);
    nuevoLogger->program_name = strdup("CPU-KERNEL");

    escucharAlKernel(); //Misma duda...
	
	return 0;
}	