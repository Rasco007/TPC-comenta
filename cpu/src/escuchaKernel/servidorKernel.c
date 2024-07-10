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

int noEsBloqueante(t_comando instruccionActual) {
	t_comando instruccionesBloqueantes[13] = {
		IO_FS_CREATE, IO_FS_DELETE, IO_FS_READ, IO_FS_TRUNCATE,
		IO_FS_WRITE, IO_GEN_SLEEP, IO_STDIN_READ, IO_STDOUT_WRITE, WAIT, SIGNAL 
	};

	for (int i = 0; i < 13; i++) 
		if (instruccionActual == instruccionesBloqueantes[i]) return 0;

	return 1;
}


//CPU recibe instrucciones del Kernel para hacer el ciclo de instruccion
int ejecutarServidorCPU(int socketCliente){
	while(1){
    instruccionActual = -1;
	int codOP = recibirOperacion(socketCliente);
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
						list_clean_and_destroy_elements(contextoEjecucion->tablaDePaginas, free);
					}
					log_info(logger,"APAREZCO DESDE CPU");
					recibirContextoBeta(socketCliente);
					//Inicio el cronometro del tiempo de uso de CPU
					log_info(logger,"luego de recibir contexto de kernel");
					contextoEjecucion->tiempoDeUsoCPU=temporal_create(); //CREO EL CRONOMETRO
					/*log_info(logger,"-*-*- Antes del while, programCounter: %d",contextoEjecucion->programCounter);
					log_info(logger,"-*-*- Antes del while, contextoEjecucion->instruccionesLength: %d",contextoEjecucion->instruccionesLength);*/
					log_info(logger,"-*-*- InstruccionesLength: %d",contextoEjecucion->instruccionesLength);
					log_info(logger,"-*-*- Ejecutando instruccion %d",contextoEjecucion->programCounter);
					// 	cicloDeInstruccion();
					// contextoEjecucion->instruccionesLength = 5; //TODO: Quitar nro magico
					 
					 flag_bloqueante = 0;
					 while(contextoEjecucion->programCounter < 3 && flag_bloqueante == 0/*&& contextoEjecucion->programCounter < contextoEjecucion->instruccionesLength*/) {
					 	
						log_info(logger,"-*-*- Ejecutando instruccion %d",contextoEjecucion->programCounter);
					 	cicloDeInstruccion();
					 } 
					//temporal_destroy (rafagaCPU);
					break;
				default:
					log_warning(loggerError,"Operacion desconocida.");
						break;
			}
	}
	return EXIT_SUCCESS;
}

