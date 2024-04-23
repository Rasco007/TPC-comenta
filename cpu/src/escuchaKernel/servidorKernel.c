#include <escuchaKernel/servidorKernel.h>

int ejecutarServidorCPU();
t_contexto *recibirPCB();

void escucharAlKernel() {
    char *puertoEscuchaDispatch = confGet("PUERTO_ESCUCHA_DISPATCH");

    log_info(logger,"Esperando conexiones CPU...");
    
    int socketClienteDispatch = alistarServidor(puertoEscuchaDispatch);
	log_info(logger, "Kernel conectado (dispatch), en socket: %d",socketClienteDispatch);
    
    char *puertoEscuchaInterrupt = confGet("PUERTO_ESCUCHA_INTERRUPT"); 
    int socketClienteInterrupt = alistarServidor(puertoEscuchaInterrupt);
	log_info(logger, "Kernel conectado (interrupt), en socket: %d", socketClienteInterrupt);

    log_info(logger,"Conexiones CPU-Kernel OK!");
    ejecutarServidorCPU();
}


int ejecutarServidorCPU(){
	return 0;
}

