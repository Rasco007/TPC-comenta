#include <escuchaKernel/servidorKernel.h>

int socketClienteDispatch;
int socketClienteInterrupt;

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
    instruccionActual = -1;
	int codOP = recibirOperacion(socketCliente);
	while(1){
		switch (codOP) {
				case MENSAJE:
					log_info(logger, "Mensaje recibido.");
					char* mensaje = recibirMensaje(socketCliente);
					log_info(logger, "Mensaje: %s", mensaje);
					break;
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
					recibirContextoBeta(socketCliente);
					//Inicio el cronometro del tiempo de uso de CPU
					log_info(logger,"llego hasta cax2");
					contextoEjecucion->tiempoDeUsoCPU=temporal_create();
					/*log_info(logger,"-*-*- Antes del while, programCounter: %d",contextoEjecucion->programCounter);
					log_info(logger,"-*-*- Antes del while, contextoEjecucion->instruccionesLength: %d",contextoEjecucion->instruccionesLength);*/
					log_info(logger,"-*-*- InstruccionesLength: %d",contextoEjecucion->instruccionesLength);
					contextoEjecucion->instruccionesLength = 5; //TODO: Quitar nro magico
					while(contextoEjecucion->programCounter < (int) contextoEjecucion->instruccionesLength /*&& contextoEjecucion->programCounter < contextoEjecucion->instruccionesLength*/) {
						log_info(logger,"-*-*- Ejecutando instruccion %d",contextoEjecucion->programCounter);
						cicloDeInstruccion();
					} //NOTA: esto lo sacamos porque es del otro tp, no tiene que ser asi aca.
					temporal_destroy (rafagaCPU);
					break;
				default:
					log_warning(loggerError,"Operacion desconocida.");
						break;
			}
	}
	return EXIT_SUCCESS;
}

