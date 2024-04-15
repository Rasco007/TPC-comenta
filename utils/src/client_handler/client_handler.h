#ifndef CLIENT_HANDLER_H_
#define CLIENT_HANDLER_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<string.h>
#include<stdio.h>
#include<communication/communication.h>
#include<connections/connection.h>
#include <commons/string.h>

static const char* NOMBRE_CLIENTE_CPU_DISPATCH = "CPU_DISPATCH";
static const char* NOMBRE_CLIENTE_CPU_INTERRUPT = "CPU_INTERRUPT";
static const char* NOMBRE_CLIENTE_MEMORIA = "MEMORIA";

int startCliente(char *ip, char* puerto, const char* nombreClient, tHsCode clientHandShake, tHsCode serverHandShakeExpected, t_log* logger);

#endif /* CLIENT_HANDLER_H_ */
