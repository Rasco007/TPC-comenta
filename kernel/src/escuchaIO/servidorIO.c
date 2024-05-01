#include <escuchaIO/servidorIO.h>

int ejecutarServidorKernel();

void escucharAlIO() {
    char *puertoEscucha = confGet("PUERTO_ESCUCHA");

    log_info(logger,"Esperando conexiones con IO...");
    
    int socketClienteIO = alistarServidor(puertoEscucha);
	log_info(logger, "IO conectado, en socket: %d",socketClienteIO);
    
    // char *puertoEscuchaInterrupt = confGet("PUERTO_ESCUCHA_INTERRUPT"); 
    // int socketClienteInterrupt = alistarServidor(puertoEscuchaInterrupt);
	// log_info(logger, "IO conectado (interrupt), en socket: %d", socketClienteInterrupt);

    log_info(logger,"Conexiones IO-Kernel OK!");
    ejecutarServidorKernel();
}


int ejecutarServidorKernel(){
	return 0;
}

