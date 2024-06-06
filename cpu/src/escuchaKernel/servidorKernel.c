#include <escuchaKernel/servidorKernel.h>

int ejecutarServidorCPU();
t_contexto *recibirPCB(); //???

void escucharAlKernel() {
    char *puertoEscuchaDispatch = confGet("PUERTO_ESCUCHA_DISPATCH");

    log_info(logger,"Esperando conexiones con KERNEL...");
    
    int socketClienteDispatch = alistarServidor(puertoEscuchaDispatch);
	log_info(logger, "Kernel conectado (dispatch), en socket: %d",socketClienteDispatch);
    
    char *puertoEscuchaInterrupt = confGet("PUERTO_ESCUCHA_INTERRUPT"); 
    int socketClienteInterrupt = alistarServidor(puertoEscuchaInterrupt);
	log_info(logger, "Kernel conectado (interrupt), en socket: %d", socketClienteInterrupt);

    log_info(logger,"Conexiones CPU-Kernel OK!");
    ejecutarServidorCPU(socketClienteDispatch,socketClienteInterrupt);
}


//CPU recibe instrucciones del Kernel para hacer el ciclo de instruccion
int ejecutarServidorCPU(int socketClienteDispatch,int socketClienteInterrupt){
    instruccionActual = -1;
	int codOP = recibirOperacion(socketClienteDispatch);
	switch (codOP) {
		case -1:
			log_info(logger, "El Kernel se desconecto.");
			if (contextoEjecucion != NULL){
				destroyContexto ();
			}
			return EXIT_FAILURE;
		case CONTEXTOEJECUCION:
			if (contextoEjecucion != NULL){
				list_clean_and_destroy_elements (contextoEjecucion->instrucciones, free);
			} 		
				recibirContextoActualizado(socketClienteDispatch);
    			rafagaCPU = temporal_create(); 
                while(contextoEjecucion->programCounter != (int) contextoEjecucion->instruccionesLength) {
                    cicloDeInstruccion(socketClienteInterrupt,socketClienteDispatch);
                }	
				temporal_destroy (rafagaCPU);
				break;
		default:
			log_warning(loggerError,"Operacion desconocida.");
				break;
	}
	return EXIT_SUCCESS;
}

