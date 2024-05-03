#include <escuchaKernel/servidorKernel.h>

int ejecutarServidorCPU();
t_contexto *recibirPCB(); //ver

void escucharAlKernel() {
    char *puertoEscuchaDispatch = confGet("PUERTO_ESCUCHA_DISPATCH");

    log_info(logger,"Esperando conexiones con KERNEL...");
    
    int socketClienteDispatch = alistarServidor(puertoEscuchaDispatch);
	log_info(logger, "Kernel conectado (dispatch), en socket: %d",socketClienteDispatch);
    
    char *puertoEscuchaInterrupt = confGet("PUERTO_ESCUCHA_INTERRUPT"); 
    int socketClienteInterrupt = alistarServidor(puertoEscuchaInterrupt);
	log_info(logger, "Kernel conectado (interrupt), en socket: %d", socketClienteInterrupt);

    log_info(logger,"Conexiones CPU-Kernel OK!");
    ejecutarServidorCPU();
}


bool noEsBloqueante(t_comando instruccionActual) {
	t_comando instruccionesBloqueantes[10] = {
	//TODO: Completar instrucciones bloqueantes
	};

	for (int i = 0; i < 10; i++) 
		if (instruccionActual == instruccionesBloqueantes[i]) return false;

	return true;
}

int ejecutarServidorCPU(){
    instruccionActual = -1;
	int codOP = recibirOperacion(socketCliente);
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
				recibirContextoActualizado(socketCliente);
    			rafagaCPU = temporal_create(); 
                while(contextoEjecucion->programCounter != (int) contextoEjecucion->instruccionesLength 
					  && (noEsBloqueante(instruccionActual))) {
                    cicloDeInstruccion();
                }	
				temporal_destroy (rafagaCPU);
				break;
		default:
			log_warning(loggerError,"Operacion desconocida.");
				break;
	}
	return EXIT_SUCCESS;
}

