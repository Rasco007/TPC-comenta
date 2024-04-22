#include <conexionMemoria/conexionMemoria.h>

void conexionMemoria() {
    char *puertoEscucha = confGet("PUERTO_MEMORIA"); 
    int socketCliente = alistarServidor(puertoEscucha);
	log_info(logger, "Conexion con Memoria exitosa");
}
