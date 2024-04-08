#ifndef SERVER_HANDLER_H_
#define SERVER_HANDLER_H_

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

int startServer(char *ip, char* puerto, char* nombreServer, hs_code serverHandShake, hs_code clienteHandShakeExpected, t_log* logger);

#endif /* SERVER_HANDLER_H_ */
