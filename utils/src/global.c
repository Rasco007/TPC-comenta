
#include <global.h>
#include <./utilsCliente/utilsCliente.h>
#include <./utilsServidor/utilsServidor.h>
#include <./contextoEjecucion/contextoEjecucion.h>

void dormirbeta( char* mensaje, int entero, int socket_cliente) {
    // Asignar memoria para el paquete
    t_paquetebeta* paquete = malloc(sizeof(t_paquete));

    // Asignar el código de operación al paquete
    paquete->codigo_operacion = IO_GEN_SLEEP;

    // Asignar memoria para el buffer
    paquete->buffer = malloc(sizeof(t_buffer));

    // Copiar el mensaje al buffer
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    // Asignar el entero al paquete
    paquete->entero = entero;

    // Calcular el tamaño total del paquete
    int bytes = sizeof(op_code) + sizeof(int) + paquete->buffer->size;

    // Serializar el paquete
    void* a_enviar = malloc(bytes);
    memcpy(a_enviar, &(paquete->codigo_operacion), sizeof(op_code));
    memcpy(a_enviar + sizeof(op_code), &(paquete->entero), sizeof(int));
    memcpy(a_enviar + sizeof(op_code) + sizeof(int), paquete->buffer->stream, paquete->buffer->size);

    // Enviar el paquete a través del socket
    send(socket_cliente, a_enviar, bytes, 0);
    log_info(logger, "La interfaz '%s' dormirá durante %d unidades de tiempo", mensaje, entero);
    // Liberar la memoria asignada
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
}
