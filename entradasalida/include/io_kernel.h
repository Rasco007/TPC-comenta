#ifndef IO_KERNEL_H_
#define IO_KERNEL_H_

#include "../src/main/entradasalida.h"

#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <configuraciones/configuraciones.h>

void io_atender_kernel();
void recibir_mensaje_y_dormir(int socket_cliente); //lo habia puesto en utilsServidor
void manejarSTDINRead(int socketCliente);
#endif
