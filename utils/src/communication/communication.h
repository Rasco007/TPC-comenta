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

#endif /* COMMUNICATION_H_ */
