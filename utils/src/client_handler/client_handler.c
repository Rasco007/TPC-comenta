#include <client_handler/client_handler.h>

int startCliente(char *ip, char* puerto, char* nombreClient, tHsCode clientHandShake, tHsCode serverHandShakeExpected, t_log* logger) {
	
	// Creamos una conexión hacia el servidor
	int socketCliente = crearConexion(ip, puerto);

	if (socketCliente == -1) {
		return -1;
	}

	char* textoCreadaConexion = string_from_format("Creada la conexion desde %d", socketCliente);
	log_info(logger, textoCreadaConexion);

	tHsCode result = handleClientHandShaking(socketCliente, clientHandShake, serverHandShakeExpected, logger);

	if (result == HS_OK)
    {
        log_info(logger, "Handshake valido - Conexion aceptada");
        return socketCliente;
    }
    else
    {
        log_warning(logger, "Handshake invalido - Conexion rechazada");
        return 0;
    }
}
