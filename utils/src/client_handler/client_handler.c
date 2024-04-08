#include <client_handler/client_handler.h>

int startCliente(char *ip, char* puerto, char* nombreClient, hs_code clientHandShake, hs_code serverHandShakeExpected, t_log* logger) {
	
	// Creamos una conexión hacia el servidor
	int socketCliente = crearConexion(ip, puerto);

	if (socketCliente == -1) {
		return -1;
	}

	char* textoCreadaConexion = string_from_format("Creada la conexion desde %d", socketCliente);
	log_info(logger, textoCreadaConexion);

	hs_code result = handleClientHandShaking(socketCliente, clientHandShake, serverHandShakeExpected, logger);

	return (result == HS_OK) ? 0 : -1;
}
