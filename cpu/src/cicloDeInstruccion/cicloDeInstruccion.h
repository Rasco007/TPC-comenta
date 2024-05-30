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

    uint32_t mmu(char* direccionLogica, int tamValor);
    char* recibirValor(int);
    void destruirTemporizador (t_temporal * temporizador);
    void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3);
    int obtenerTamanioReg(char* registro);

#endif 