#ifndef MMU_H_
#define MMU_H_



    #include <utilsCliente/utilsCliente.h>
	#include <stdint.h>
	#include <conexionMemoria/conexionMemoria.h>
	#include <cicloDeInstruccion/cicloDeInstruccion.h>

    #define obtenerAlgoritmoTLB() config_get_string_value(config, "ALGORITMO_TLB")
	#define CANTIDAD_ENTRADAS_TLB 32 // Define la cantidad de entradas en la TLB
    #define PAGE_SIZE 2048 // Define el tamaño de una página

    typedef struct {
        uint32_t pid;         // ID del proceso
        uint32_t page_number; // Número de página
        uint32_t frame_number;// Número de marco
        bool valid;           // Validez de la entrada
        uint64_t last_used;   // Timestamp para LRU (Opcional, para el manejo de LRU)
    } TLBEntry;

    typedef struct {
        TLBEntry entries[CANTIDAD_ENTRADAS_TLB];
        size_t size;
        char* algoritmo;
    } TLB;

    
	uint32_t mmu(uint32_t pid, char* direccionLogica, int tamValor);
	//bool manejar_fallo_de_pagina(PageTable *page_table, uint32_t page_number, uint32_t frame_number);
    void inicializar_tlb(char* algoritmoTLB);
    int consultar_tlb(uint32_t pid, uint32_t page_number, uint32_t *frame_number);
    void agregar_a_tlb(uint32_t pid, uint32_t page_number, uint32_t frame_number);

    void solicitarDireccion(int pid, int pagina, int socket);
    void limpiarBuffer(int socketCliente);
#endif
