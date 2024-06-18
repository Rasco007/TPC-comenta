#ifndef MMU_H_
#define MMU_H_



    #include <utilsCliente/utilsCliente.h>
	#include <stdint.h>
	#include <conexionMemoria/conexionMemoria.h>
	#include <cicloDeInstruccion/cicloDeInstruccion.h>


	#define PAGE_SIZE 2048

    typedef struct {
    uint32_t frame_number;
    bool valid;
    } PageTableEntry;

    typedef struct {
    PageTableEntry *entries;
    size_t size;
    } PageTable;

    typedef struct {
    uint32_t pid;         // ID del proceso
    uint32_t page_number; // Número de página
    uint32_t frame_number;// Número de marco
    bool valid;           // Validez de la entrada
    uint64_t last_used;   // Timestamp para LRU
    } TLBEntry;
    typedef struct {
    TLBEntry entries[32];
    size_t size;
    } TLB;

	uint32_t mmu(char* direccionLogica, int tamValor, TLB tlb);
	bool manejar_fallo_de_pagina(PageTable *page_table, uint32_t page_number, uint32_t frame_number);
    void inicializar_tlb();
    void inicializar_tabla_paginas(size_t num_pages);


#endif
