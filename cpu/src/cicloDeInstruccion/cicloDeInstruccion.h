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

    extern char *listaComandos[];
    extern char* instruccionAEjecutar; 
    extern char** elementosInstruccion; 
    extern int instruccionActual; 
    extern int cantParametros;
    extern int tiempoEspera;
    extern int conexionAMemoria;

    extern t_contexto* contextoEjecucion;

    void cicloDeInstruccion();
    void fetch();
    void decode();
    void execute();
    void liberarMemoria();
    int buscar(char *elemento, char **lista); 

    void set_c(char*, char*);
    void yield_c(); 
    void exit_c(); 
    void mov_in(char*, char*);
    void mov_out(char*, char*);
    void wait_c(char*);
    void signal_c(char* );

    uint32_t mmu(char* direccionLogica, int tamValor);
    char* recibirValor(int);
    void destruirTemporizador (t_temporal * temporizador);
    void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3, char * parm4, char * parm5);
    int obtenerTamanioReg(char* registro);

#endif 