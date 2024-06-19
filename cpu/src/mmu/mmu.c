#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

extern TLB tlb;
extern uint64_t tiempo_actual; // Contador de tiempo para LRU

void solicitarDireccion(int socket){
    /*t_paquete* peticion = crearPaquete();
    peticion->codigo_operacion = MMU;
    
    enviarPaquete(peticion, conexionAMemoria);    
    eliminarPaquete (peticion);*/
    t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = MMU;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	//memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializarPaquete(paquete, bytes);

	send(socket, a_enviar, bytes, 0);

	free(a_enviar);
	eliminarPaquete(paquete);
}
void limpiarBuffer(int socketCliente){
    int size;
    void* buffer = recibirBuffer(socketCliente, &size);
    free(buffer);

}
//MMU
uint32_t mmu(uint32_t pid, char* direccionLogica, int tamValor) {
    uint32_t dirLogica = (uint32_t)strtoul(direccionLogica, NULL, 10);
    uint32_t page_number = dirLogica / PAGE_SIZE;
    uint32_t offset = dirLogica % PAGE_SIZE;
    int recibo;
    char* valorAInsertar;

    uint32_t frame_number;
    if (consultar_tlb(pid, page_number, &frame_number)) {
        // TLB Hit
        return frame_number * PAGE_SIZE + offset;
    } else {
        // TLB Miss
        // Aquí deberías consultar la tabla de páginas en memoria y actualizar la TLB
        // Por ahora, solo indicamos un TLB Miss
        log_info(logger,"TLB Miss\n");
        solicitarDireccion(conexionAMemoria);
        /*t_paquete* peticion = crearPaquete();
        peticion->codigo_operacion = MMU;
        
        enviarPaquete(peticion, conexionAMemoria);    */
        //eliminarPaquete (peticion);
        
        
        int control = 1;
        while(control) {
        recibo = recibirOperacion(conexionAMemoria);
        log_info(logger,"numero de case??? %d", recibo);
        switch (recibo){
            case MMU:
                valorAInsertar = recibirMensaje(conexionAMemoria);
                log_info(logger,"valorAInsertar de memoria: %s\n", valorAInsertar);
                control=0;
            break;
        
            default:
                log_warning(logger, "Operación desconocida de la memoria.");   
                valorAInsertar = recibirMensaje(conexionAMemoria);
                log_info(logger,"valorAInsertar de memoria: %s\n", valorAInsertar);
                limpiarBuffer(conexionAMemoria);
                control=0;
            break;
            case -1:
                log_error(logger, "ERROR OPCODE");
                return EXIT_FAILURE;
                break;
            }
        }
        
        /*log_info(logger,"Recibo de memoria: %d\n", recibo);

        valorAInsertar = recibirMensaje(conexionAMemoria);

        log_info(logger,"valorAInsertar de memoria: %s\n", valorAInsertar);*/

        return UINT32_MAX; // Indica que no se encontró
    }
}

void inicializar_tlb() {
    tlb.size = CANTIDAD_ENTRADAS_TLB; // Establece el tamaño de la TLB
    for (size_t i = 0; i < CANTIDAD_ENTRADAS_TLB; i++) {
        tlb.entries[i].pid = 0;          // ID de proceso inicializado a 0
        tlb.entries[i].page_number = 0;  // Número de página inicializado a 0
        tlb.entries[i].frame_number = 0; // Número de marco inicializado a 0
        tlb.entries[i].valid = false;    // Marca la entrada como inválida
        tlb.entries[i].last_used = 0;    // Timestamp inicializado a 0 (opcional)
    }
    log_info(logger,"TLB inicializada.");
}

int consultar_tlb(uint32_t pid, uint32_t page_number, uint32_t *frame_number) {
    for (size_t i = 0; i < tlb.size; i++) {
        if (tlb.entries[i].valid && tlb.entries[i].pid == pid && tlb.entries[i].page_number == page_number) {
            // TLB Hit
            tlb.entries[i].last_used = tiempo_actual++;
            *frame_number = tlb.entries[i].frame_number;
            return 1; // Indica TLB Hit
        }
    }
    // TLB Miss
    return 0; // Indica TLB Miss
}