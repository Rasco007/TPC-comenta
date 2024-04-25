#include <escuchaKernel/servidorKernel.h>

int ejecutarServidorCPU();
t_contexto *recibirPCB();

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


int ejecutarServidorCPU(){
    instruccionActual = -1;
		int codOP = recibirOperacion(socketCliente);
		switch (codOP) {
			case -1:
				log_info(logger, "El Kernel se desconecto.");
				if (contextoEjecucion != NULL)
					destroyContexto ();
				return EXIT_FAILURE;
			case CONTEXTOEJECUCION:
				if (contextoEjecucion != NULL) 
					list_clean_and_destroy_elements (contextoEjecucion->instrucciones, free),
					list_clean_and_destroy_elements (contextoEjecucion->tablaDeSegmentos, free);
				recibirContextoActualizado(socketCliente);
    			rafagaCPU = temporal_create(); 
                while(contextoEjecucion->programCounter != (int) contextoEjecucion->instruccionesLength 
					  && (noEsBloqueante(instruccionActual))) {
                    cicloDeInstruccion();
                }	
				temporal_destroy (rafagaCPU);
				break;
			default:
				log_warning(loggerError,"Operacion desconocida. No quieras meter la pata");
				break;
		}
}

