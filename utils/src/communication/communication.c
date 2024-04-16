#include <communication/communication.h>

tHsCode handleClientHandShaking(int socketCliente, tHsCode clientHandShake, tHsCode serverHandShakeExpected, t_log* logger) {

    tHsCode serverHandShakeResponse;
    tHsCode hsResult;

    //mando quien soy
	send(socketCliente, &clientHandShake, sizeof(int), 0);

	//recibo quien es el otro
	recv(socketCliente, &serverHandShakeResponse, sizeof(int), MSG_WAITALL);
    
    //Si todo fue bien, mando un ok para ver si el server era el que esperaba y que el cliente no sea rechazado.
	if (serverHandShakeResponse == serverHandShakeExpected) {
		log_info(logger, "Handshaking okey entre cliente y servidor. Conexion aceptada");
		hsResult = HS_OK;
    } else {
		log_info(logger, "Handshaking fallo entre cliente y servidor. Conexion rechazada");
		hsResult = HS_FAIL;
	}

	send(socketCliente, &hsResult, sizeof(int), 0);

    return hsResult;
}

tHsCode handleServerHandShaking(int socketServer, tHsCode serverHandShake, tHsCode clientHandShakeRequestExpected, t_log* logger) {

    tHsCode serverHandShakeResponse;
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

void* serializar_paquete(tPaquete* paquete, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente){
	tPaquete* paquete = malloc(sizeof(tPaquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(tBuffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void eliminar_paquete(tPaquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

int recibir_operacion(int socket_cliente){
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente){
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(t_log* logger, int socket_cliente){
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente){
	int size;
	int desplazamiento = 0;
	void* buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size){
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void crear_buffer(tPaquete* paquete){
	paquete->buffer = malloc(sizeof(tBuffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

tPaquete* crear_paquete(tOpCode codigo_op){
	tPaquete* paquete = malloc(sizeof(tPaquete));
	paquete->codigo_operacion = codigo_op;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(tPaquete* paquete, void* valor, int tamanio){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(tPaquete* paquete, int socket_cliente){
	int bytes = paquete->buffer->size + sizeof(int) + sizeof(tOpCode);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}