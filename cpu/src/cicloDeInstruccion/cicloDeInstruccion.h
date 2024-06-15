#ifndef CICLODEINSTRUCCION_H
#define CICLODEINSTRUCCION_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <unistd.h>
    #include <commons/string.h>
    #include <commons/config.h>
    #include <commons/log.h>
    #include <commons/temporal.h>
    #include <commons/collections/list.h>
    #include <global.h>
    #include <configuraciones/configuraciones.h>
    #include <utilsCliente/utilsCliente.h>
    #include <utilsServidor/utilsServidor.h>
    #include <contextoEjecucion/contextoEjecucion.h>
    #include <conexionMemoria/conexionMemoria.h>
    #include <escuchaKernel/servidorKernel.h>

    #define obtenerTiempoEspera() config_get_int_value(config, "RETARDO_INSTRUCCION")
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

    

    extern char *listaComandos[];
    extern char* instruccionAEjecutar; 
    extern char** elementosInstruccion; 
    extern int instruccionActual; 
    extern int cantParametros;
    extern int tiempoEspera;
    extern int conexionAMemoria;

    extern t_contexto* contextoEjecucion;

    void cicloDeInstruccion(int socket1,int socket2);
    void fetch();
    void decode();
    void execute();
    void liberarMemoria();
    void check_interrupt(int socket1,int socket2);
    int buscar(char *elemento, char **lista); 

    uint32_t mmu(char* direccionLogica, int tamValor, TLB tlb);
    char* recibirValor(int);
    void destruirTemporizador (t_temporal * temporizador);
    void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3, char * parm4, char * parm5);
    int obtenerTamanioReg(char* registro);
    bool manejar_fallo_de_pagina(PageTable *page_table, uint32_t page_number, uint32_t frame_number);
    void inicializar_tlb();
    void inicializar_tabla_paginas(size_t num_pages);

    
    
    //
    

#endif 