#include <communication/communication.h>

hs_code handleClientHandShaking(int socketCliente, hs_code clientHandShake, hs_code serverHandShakeExpected, t_log* logger) {

    hs_code serverHandShakeResponse;
    hs_code hsResult;

    //mando quien soy
	send(socketCliente, &clientHandShake, sizeof(int), 0);

	//recibo quien es el otro
	recv(socketCliente, &serverHandShakeResponse, sizeof(int), MSG_WAITALL);
    
    //Si todo fue bien, mando un ok para ver si el server era el que esperaba y que el cliente no sea rechazado.
	if (serverHandShakeResponse == serverHandShakeExpected) {
		log_info(logger, "Handshaking okey entre cliente y servidor. Conexion aceptada");
		hsResult = HS_OK;
		send(socketCliente, &hsResult, sizeof(int), 0);
    } else {
		log_info(logger, "Handshaking fallo entre cliente y servidor. Conexion rechazada");
		hsResult = HS_FAIL;
		send(socketCliente, &hsResult, sizeof(int), 0);
	}

    return hsResult;
}

hs_code handleServerHandShaking(int socketServer, hs_code serverHandShake, hs_code clientHandShakeRequestExpected, t_log* logger) {

    hs_code serverHandShakeResponse;
    hs_code hsResult;
    int clientHandShakeRequest = -1;
	int isValidConnection = -1;

    //handshaking handling
	recv(socketServer, &clientHandShakeRequest, sizeof(int), MSG_WAITALL);

	char* textoHandShakeRequest = string_from_format("HS Request %d", clientHandShakeRequest);
	log_info(logger, textoHandShakeRequest);
	
    if (clientHandShakeRequestExpected == clientHandShakeRequest) {
		send(socketServer, &serverHandShakeResponse, sizeof(int), MSG_WAITALL);
	}

	recv(socketServer, &isValidConnection, sizeof(int), MSG_WAITALL);

	if (isValidConnection == HS_OK) {
		log_info(logger, "Valid connection");
	} else {
		log_info(logger, "Invalid connection");
	}


    return isValidConnection;
}