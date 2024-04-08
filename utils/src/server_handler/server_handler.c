#include <server_handler/server_handler.h>

int startServer(char *ip, char* puerto, char* nombreServer, hs_code serverHandShake, hs_code clienteHandShakeExpected, t_log* logger) {
	
	// Creamos un socket hacia el servidor
	int socketServer = iniciarServidor(logger, ip, puerto);

	if (socketServer == -1) {
		log_error(logger, "Fallo el levantado de conexion del server");
		return -1;
	}

	log_info(logger, "Servidor listo para recibir al cliente");
	
	int socketCliente = esperarCliente(logger, socketServer, nombreServer);

	char* textoSeConectoNuevoCliente = string_from_format("Se conecto un cliente %d", socketCliente);
	log_info(logger, textoSeConectoNuevoCliente);

	hs_code result = handleClientHandShaking(socketCliente, clienteHandShakeExpected, serverHandShake, logger);

	return (result == HS_OK) ? 0 : -1;
}
