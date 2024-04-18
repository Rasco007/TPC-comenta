#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<string.h>
#include<stdio.h>
#include <commons/string.h>
#include<commons/collections/list.h>

//handshaking

typedef enum
{
    HS_OK,
    HS_FAIL,
    HS_KERNEL,
    HS_CPU_DISPATCH,
    HS_CPU_INTERRUPT,
    HS_MEMORIA,
    HS_IO
} tHsCode; //Enum para identificar codigos de Hand Shaking

tHsCode handleClientHandShaking(int socketCliente, tHsCode clientHandShake, tHsCode serverHandShakeExpected, t_log* logger);
tHsCode handleServerHandShaking(int socketServer, tHsCode serverHandShake, tHsCode clientHandShakeRequestExpected, t_log* logger);

//transmision de mensajes

typedef enum{
	MENSAJE,
	PAQUETE
} tOpCode;

typedef struct{
	int size;
	void* stream;
} tBuffer;

typedef struct{
	tOpCode codigo_operacion;
	tBuffer* buffer;
} tPaquete;

//Mensajes
void enviar_mensaje(char* mensaje, int socket_cliente);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(t_log* logger, int socket_cliente);
void crear_buffer(tPaquete* paquete);

//Paquetes
t_list* recibir_paquete(int);
tPaquete* crear_paquete(tOpCode);
void agregar_a_paquete(tPaquete* paquete, void* valor, int tamanio);
void agregar_a_paquete_con_header(tPaquete *paquete, void *valor, int tamanio);
void enviar_paquete(tPaquete* paquete, int socket_cliente);
void eliminar_paquete(tPaquete* paquete);
void* serializar_paquete(tPaquete* paquete, int bytes);

#endif /* COMMUNICATION_H_ */