#ifndef CONNECTION_H_
#define CONNECTION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<string.h>

int iniciarServidor(t_log* logger, char* ip, char* puerto);
int esperarCliente(t_log* logger, const char* name, int socket_servidor);
int crearConexion(char *ip, char* puerto);
void liberarConexion(int socket_cliente);

#endif /* CONNECTION_H_ */
