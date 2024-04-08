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
    HS_CPU,
    HS_MEMORIA,
    HS_IO
} hs_code; //Enum para identificar codigos de Hand Shaking

hs_code handleClientHandShaking(int socketCliente, hs_code clientHandShake, hs_code serverHandShakeExpected, t_log* logger);
hs_code handleServerHandShaking(int socketServer, hs_code serverHandShake, hs_code clientHandShakeRequestExpected, t_log* logger);

//transmision de mensajes

typedef enum{
	MENSAJE,
	PAQUETE
} op_code;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

//Mensajes
void enviar_mensaje(char* mensaje, int socket_cliente);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(t_log* logger, int socket_cliente);
void crear_buffer(t_paquete* paquete);

//Paquetes
t_list* recibir_paquete(int);
t_paquete* crear_paquete(op_code);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void agregar_a_paquete_con_header(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);

#endif /* COMMUNICATION_H_ */
