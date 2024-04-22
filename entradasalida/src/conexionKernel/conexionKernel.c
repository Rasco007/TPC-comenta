#include <conexionKernel/conexionKernel.h>

void conexionKernel() {
    char *puertoEscucha = confGet("PUERTO_KERNEL"); 
    int socketCliente = alistarServidor(puertoEscucha);
	log_info(logger, "Conexion con Kernel exitosa");
}

