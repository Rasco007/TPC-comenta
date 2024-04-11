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

int startCliente(char *ip, char* puerto, char* nombreClient, hs_code clientHandShake, hs_code serverHandShakeExpected, t_log* logger);

#endif /* CLIENT_HANDLER_H_ */
