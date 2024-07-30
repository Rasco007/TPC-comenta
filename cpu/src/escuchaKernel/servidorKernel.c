#include <escuchaKernel/servidorKernel.h>
#include <pthread.h>


int socketClienteDispatch;
int socketClienteInterrupt;
t_temporal* tiempoDeUsoCPU;

void escucharAlKernel() {
    char *puertoEscuchaDispatch = confGet("PUERTO_ESCUCHA_DISPATCH");

    log_info(logger,"Esperando conexiones con KERNEL...");
    
    socketClienteDispatch = alistarServidor(puertoEscuchaDispatch);
	log_info(logger, "Kernel conectado (dispatch), en socket: %d",socketClienteDispatch);
    
    char *puertoEscuchaInterrupt = confGet("PUERTO_ESCUCHA_INTERRUPT"); 
    socketClienteInterrupt = alistarServidor(puertoEscuchaInterrupt);
	log_info(logger, "Kernel conectado (interrupt), en socket: %d", socketClienteInterrupt);

    log_info(logger,"Conexiones CPU-Kernel OK!");

    ejecutarServidorCPU(socketClienteDispatch);
}



//CPU recibe instrucciones del Kernel para hacer el ciclo de instruccion
int ejecutarServidorCPU(int socketCliente){
	pthread_t atenderInterrupcionKernel;
	pthread_create(&atenderInterrupcionKernel,NULL, (void*) atenderInterrupcionDeUsuario,NULL);
	while(1){
    instruccionActual = -1;
	int codOP = recibirOperacion(socketCliente);
		switch (codOP) {
				case -1:
					log_error(logger, "El Kernel se desconecto.");
					if (contextoEjecucion != NULL){
						destroyContexto ();
					}				
					return EXIT_FAILURE;
				case CONTEXTOEJECUCION:
					log_info(logger,"APAREZCO DESDE CPU");
					recibirContextoBeta(socketCliente);
					//Inicio el cronometro del tiempo de uso de CPU
					log_info(logger,"luego de recibir contexto de kernel");
					tiempoDeUsoCPU=temporal_create(); //Inicio y creo el CRONOMETRO
					/*log_info(logger,"-*-*- Antes del while, programCounter: %d",contextoEjecucion->programCounter);
					log_info(logger,"-*-*- Antes del while, contextoEjecucion->instruccionesLength: %d",contextoEjecucion->instruccionesLength);*/
					log_info(logger,"-*-*- InstruccionesLength: %d",contextoEjecucion->instruccionesLength);
					log_info(logger,"-*-*- Ejecutando instruccion %d",contextoEjecucion->programCounter);
					 
					 flag_bloqueante = 0;
					 while(contextoEjecucion->programCounter < contextoEjecucion->instruccionesLength && flag_bloqueante == 0) {
					 	
						//log_info(logger,"-*-*- Ejecutando instruccion %d",contextoEjecucion->programCounter);
					 	cicloDeInstruccion();
					 }

					break;
				default:
					log_warning(loggerError,"Operacion desconocida.");
						break;
			}
	}
	pthread_join(atenderInterrupcionKernel,NULL);
	return EXIT_SUCCESS;
}

void atenderInterrupcionDeUsuario(){
	while(1){
		int peticion = recibirOperacion(socketClienteInterrupt);
		if(peticion==150){
			mensajeInterrupcion = recibirMensaje(socketClienteInterrupt);	
		}
	}
}