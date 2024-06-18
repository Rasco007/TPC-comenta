#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//MMU
uint32_t mmu(char* direccionLogica, int tamValor, TLB tlb){

    //log_info(logger, "Dirección lógica: %s - Tam valor: %d", direccionLogica, tamValor);

    uint32_t dirLogica = (uint32_t)strtoul(direccionLogica, NULL, 10);
    /*log_info(logger, "dirLogica: %u",dirLogica);*/

	t_pagina pagina;
	
	pagina.idPagina = dirLogica / PAGE_SIZE;
	

    uint32_t offset = dirLogica - pagina.idPagina * PAGE_SIZE;

    //log_info(logger, "page_number: %d - offset: %d",page_number,offset);
    


    // Consultar la TLB
    /*log_info(logger, "Consultar la TLB");
    for (size_t i = 0; i < tlb.size; i++) {
        if (tlb.entries[i].valid && tlb.entries[i].page_number == page_number) {
            // TLB Hit
            //tlb.entries[i].last_used = tiempo_actual++;
            log_info(logger, "TLB Hit");
            return tlb.entries[i].frame_number * PAGE_SIZE + offset;
        }
    }*/

    log_info(logger, "TLB Miss, consultar la tabla de páginas");

    
    // TLB Miss, consultar a la memoria para obtener el frame correspondiente a la página buscada
    t_paquete* peticion = crearPaquete();
    peticion->codigo_operacion = MMU;
	agregarPaginaAPaquete(peticion, &pagina);
    /*agregarAPaquete(peticion,&contextoEjecucion->pid, sizeof(uint32_t));
    agregarAPaquete(peticion,&dirFisica, sizeof(uint32_t));
    agregarAPaquete(peticion,&tamRegistro,sizeof(uint32_t));*/
    enviarPaquete(peticion, conexionAMemoria);    
    /*eliminarPaquete (peticion);

    recibirOperacion(conexionAMemoria);
    valorAInsertar = recibirMensaje(conexionAMemoria);*/

    // Agregar entrada a la TLB
    /*bool entrada_agregada = false;
    for (size_t i = 0; i < tlb.size; i++) {
        if (!tlb.entries[i].valid) {
            tlb.entries[i] = (TLBEntry){.page_number = page_number, .frame_number = frame_number, .valid = true, .last_used = tiempo_actual++};
            entrada_agregada = true;
            break;
        }
    }

    if (!entrada_agregada) {
        // Reemplazo de la TLB usando LRU (o cualquier otro algoritmo)
        size_t victim_index = 0;
        uint64_t oldest_time = tlb.entries[0].last_used;
        for (size_t i = 1; i < tlb.size; i++) {
            if (tlb.entries[i].last_used < oldest_time) {
                oldest_time = tlb.entries[i].last_used;
                victim_index = i;
            }
        }
        tlb.entries[victim_index] = (TLBEntry){.page_number = page_number, .frame_number = frame_number, .valid = true, .last_used = tiempo_actual++};
    }

    return frame_number * PAGE_SIZE + offset;*/
    return UINT32_MAX;
}

bool manejar_fallo_de_pagina(PageTable *page_table, uint32_t page_number, uint32_t frame_number) {
    if (page_number >= page_table->size) {
        return false;  // Número de página fuera del rango
    }

    page_table->entries[page_number].frame_number = frame_number;
    page_table->entries[page_number].valid = true;

    return true;
}