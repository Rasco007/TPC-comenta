#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

extern TLB tlb;
extern uint64_t tiempo_actual; // Contador de tiempo para LRU


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

        t_paquete* peticion = crearPaquete();
        peticion->codigo_operacion = MMU;
        
        enviarPaquete(peticion, conexionAMemoria);    
        eliminarPaquete (peticion);

        

        recibo = recibirOperacion(conexionAMemoria);


        log_info(logger,"Recibo de memoria: %d\n", recibo);

        valorAInsertar = recibirMensaje(conexionAMemoria);

        log_info(logger,"valorAInsertar de memoria: %s\n", valorAInsertar);

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