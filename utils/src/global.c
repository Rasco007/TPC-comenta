
#include <global.h>
#include <./utilsCliente/utilsCliente.h>
#include <./utilsServidor/utilsServidor.h>
#include <./contextoEjecucion/contextoEjecucion.h>

void dormir(char* mensaje, int socket_cliente){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = IO_GEN_SLEEP;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2*sizeof(int);

    void* a_enviar = serializarPaquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminarPaquete(paquete);

}

