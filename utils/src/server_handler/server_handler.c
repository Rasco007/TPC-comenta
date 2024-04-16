#include <server_handler/server_handler.h>

int startServer(char *ip, char* puerto, t_log* logger) {
	
	// Creamos un socket hacia el servidor
	int socketServer = iniciarServidor(logger, ip, puerto);

	if (socketServer == -1) {
		log_error(logger, "Fallo el levantado de conexion del server");
		return -1;
	}

	log_info(logger, "Servidor listo para recibir al cliente");
	
	return socketServer;
}

bool isValidHandShake(int socketServer, char* nombreServer, tHsCode serverHandShake, tHsCode clienteHandShakeExpected, t_log* logger) {

	int socketCliente = esperarCliente(logger, nombreServer, socketServer);

	log_info(logger, "Se conecto un cliente a %s", nombreServer);

	tHsCode result = handleServerHandShaking(socketCliente, clienteHandShakeExpected, serverHandShake, logger);

	if (result == HS_OK)
    {
        log_info(logger, "Handshake valido - Conexion aceptada");
        return true;
    }
    else
    {
        log_warning(logger, "Handshake invalido - Conexion rechazada");
        return false;
    }
}
