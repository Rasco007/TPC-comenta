/* KERNEL- cliente | CPU - servidor*/
#include <conexiones/conexionCPU.h>


t_buffer* bufferContexto;
int conexionACPU;
int conexionACPUInterrupt;

void conexionCPU() {
    //CONEXION CPU DISPATCH
    logger = cambiarNombre(logger, "Kernel-CPU (dispatch)");
    loggerError = cambiarNombre(loggerError,"Errores Kernel-CPU (dispatch)");

    while(1){
        conexionACPU = conexion("CPU_DISPATCH");
        
        if(conexionACPU != -1){
           break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionACPU);
            sleep(5);
        }
    }

    //CONEXION CPU INTERRUPT
    logger = cambiarNombre(logger, "Kernel-CPU (interrupt)");
    loggerError = cambiarNombre(loggerError,"Errores Kernel-CPU (interrupt)");

    while(1){
        conexionACPUInterrupt = conexion("CPU_INTERRUPT");
        
        if(conexionACPUInterrupt != -1){
            break;
        }
        else {
            log_error(loggerError, "No se pudo conectar al servidor, socket %d, esperando 5 segundos y reintentando.", conexionACPUInterrupt);
            sleep(5);
        }
    }
    //log_info(logger, "Enviando HOLA!!");
    //enviarMensaje("HOLA!!", conexionACPU);
}

int recibirOperacionDeCPU(){ 
	int cod_op;
    
	if (recv(conexionACPU, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(conexionACPU);
		return -1;
	}
}

//Enviar proceso a CPU
t_contexto* procesarPCB(t_pcb* procesoEnEjecucion) {
    if (contextoEjecucion != NULL) destroyContextoUnico ();
	iniciarContexto ();
    
    bufferContexto = malloc(sizeof(t_buffer));

    asignarPCBAContexto(procesoEnEjecucion);
    // Loguear registros CPU
   
    dictionary_iterator(contextoEjecucion->registrosCPU, log_registro);

    enviarContextoBeta(conexionACPU, contextoEjecucion);

    if (recibirOperacionDeCPU() < 0) error ("Se desconecto la CPU.");

    recibirContextoBeta(conexionACPU); 

    actualizarPCB(procesoEnEjecucion);

    free(bufferContexto);
    return contextoEjecucion;
}

 void log_registro(char *key, void *value) {
        log_info(logger, "Registro %s: %s", key, (char*)value);
    }

void actualizarPCB(t_pcb* proceso){
	list_destroy_and_destroy_elements(proceso->instrucciones, free);
    proceso->instrucciones = list_duplicate(contextoEjecucion->instrucciones);
    proceso->pid = contextoEjecucion->pid;
    proceso->programCounter = contextoEjecucion->programCounter;
	dictionary_destroy_and_destroy_elements(proceso->registrosCPU, free);
    proceso->registrosCPU = registrosDelCPU(contextoEjecucion->registrosCPU);
     list_destroy_and_destroy_elements (proceso->tablaDePaginas, free);
    proceso->tablaDePaginas = list_duplicate(contextoEjecucion->tablaDePaginas);

}

void asignarPCBAContexto(t_pcb* proceso){

    list_destroy_and_destroy_elements(contextoEjecucion->instrucciones, free);
    contextoEjecucion->instrucciones = list_duplicate(proceso->instrucciones);
    contextoEjecucion->instruccionesLength = list_size(contextoEjecucion->instrucciones);
    contextoEjecucion->pid = proceso->pid;
    contextoEjecucion->programCounter = proceso->programCounter;
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    contextoEjecucion->registrosCPU = registrosDelCPU(proceso->registrosCPU);
    contextoEjecucion->tiempoDeUsoCPU=proceso->tiempoDeUsoCPU;
    contextoEjecucion->DI=proceso->DI;
    contextoEjecucion->SI=proceso->SI;
    if(contextoEjecucion->algoritmo != FIFO){
        contextoEjecucion->quantum=proceso->quantum;
 log_info(logger, "Quantum: %ld", contextoEjecucion->quantum);
    }
    
    list_destroy_and_destroy_elements (contextoEjecucion->tablaDePaginas, free);
    contextoEjecucion->tablaDePaginas = list_duplicate(proceso->tablaDePaginas);
    contextoEjecucion->tablaDePaginasSize = list_size(contextoEjecucion->tablaDePaginas);

 
  log_info(logger, "PID: %u", contextoEjecucion->pid);
    log_info(logger, "Program Counter: %d", contextoEjecucion->programCounter);
    log_info(logger, "SI: %u", contextoEjecucion->SI);
    log_info(logger, "DI: %u", contextoEjecucion->DI);
    log_info(logger, "Instrucciones Length: %u", contextoEjecucion->instruccionesLength);
   
    log_info(logger, "Algoritmo: %d", contextoEjecucion->algoritmo);

    // Loguear instrucciones
    for (int i = 0; i < list_size(contextoEjecucion->instrucciones); i++) {
        char *instruccion = list_get(contextoEjecucion->instrucciones, i);
        log_info(logger, "Instrucción %d: %s", i, instruccion);
    }

   

    // Loguear tabla de páginas
    log_info(logger, "Tabla de Páginas Size: %u", contextoEjecucion->tablaDePaginasSize);
    for (int i = 0; i < list_size(contextoEjecucion->tablaDePaginas); i++) {
        t_pagina *pagina = list_get(contextoEjecucion->tablaDePaginas, i);
        log_info(logger, "Página %d: IdPagina: %d, idFrame: %d, Bit de validez: %d",
                 i, pagina->idPagina, pagina->idFrame, pagina->bitDeValidez);
    }

    // Loguear motivo de desalojo
    log_info(logger, "Motivo de Desalojo: %d", contextoEjecucion->motivoDesalojo->motivo);
    log_info(logger, "Motivo de Desalojo Parámetros Length: %u", contextoEjecucion->motivoDesalojo->parametrosLength);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        log_info(logger, "Motivo de Desalojo Parámetro %d: %s", i, contextoEjecucion->motivoDesalojo->parametros[i]);
    }
   log_info(logger, "--------------------------------" );
}

void asignarPCBAContextoBeta(t_pcb* proceso){

t_pagina* nuevaPagina1 = malloc(sizeof(t_pagina)); 
nuevaPagina1->idPagina = 0;
nuevaPagina1->idFrame = 10;
nuevaPagina1->bitDeValidez = 1;

t_pagina* nuevaPagina2 = malloc(sizeof(t_pagina));
nuevaPagina2->idPagina = 1;
nuevaPagina2->idFrame = 20;
nuevaPagina2->bitDeValidez = 0;


    list_destroy_and_destroy_elements(contextoEjecucion->instrucciones, free);
    contextoEjecucion->instrucciones = list_duplicate(proceso->instrucciones);
    //contextoEjecucion->instrucciones = list_create();
list_add(contextoEjecucion->instrucciones, strdup("instr1"));
list_add(contextoEjecucion->instrucciones, strdup("instr2"));
list_add(contextoEjecucion->instrucciones, strdup("instr3"));
    contextoEjecucion->instruccionesLength = 3;
    contextoEjecucion->pid = proceso->pid;
    contextoEjecucion->programCounter = proceso->programCounter;
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    contextoEjecucion->registrosCPU = registrosDelCPU(proceso->registrosCPU);
    contextoEjecucion->tiempoDeUsoCPU=proceso->tiempoDeUsoCPU;
    contextoEjecucion->DI=proceso->DI;
    contextoEjecucion->SI=proceso->SI;
    contextoEjecucion->quantum=proceso->quantum;
     list_add (contextoEjecucion->tablaDePaginas, nuevaPagina1);
     list_add (contextoEjecucion->tablaDePaginas, nuevaPagina2);
 contextoEjecucion->tablaDePaginasSize=2;
}

t_dictionary *registrosDelCPU(t_dictionary *aCopiar) {
    t_dictionary *copia = dictionary_create();

    // Allocate memory for 1-byte registers
    char* AX = malloc(sizeof(char) * (1 + 1));
    char* BX = malloc(sizeof(char) * (1 + 1));
    char* CX = malloc(sizeof(char) * (1 + 1));
    char* DX = malloc(sizeof(char) * (1 + 1));

    // Allocate memory for 4-byte registers
    char* EAX = malloc(sizeof(char) * (4 + 1));
    char* EBX = malloc(sizeof(char) * (4 + 1));
    char* ECX = malloc(sizeof(char) * (4 + 1));
    char* EDX = malloc(sizeof(char) * (4 + 1));

    // Copy values from the original dictionary
    strncpy(AX, (char *)dictionary_get(aCopiar, "AX"), 1);
    strncpy(BX, (char *)dictionary_get(aCopiar, "BX"), 1);
    strncpy(CX, (char *)dictionary_get(aCopiar, "CX"), 1);
    strncpy(DX, (char *)dictionary_get(aCopiar, "DX"), 1);

    strncpy(EAX, (char *)dictionary_get(aCopiar, "EAX"), 4);
    strncpy(EBX, (char *)dictionary_get(aCopiar, "EBX"), 4);
    strncpy(ECX, (char *)dictionary_get(aCopiar, "ECX"), 4);
    strncpy(EDX, (char *)dictionary_get(aCopiar, "EDX"), 4);

    // Ensure the strings are null-terminated
    AX[1] = '\0';
    BX[1] = '\0';
    CX[1] = '\0';
    DX[1] = '\0';

    EAX[4] = '\0';
    EBX[4] = '\0';
    ECX[4] = '\0';
    EDX[4] = '\0';

    // Put values into the new dictionary
    dictionary_put(copia, "AX", AX);
    dictionary_put(copia, "BX", BX);
    dictionary_put(copia, "CX", CX);
    dictionary_put(copia, "DX", DX);
    dictionary_put(copia, "EAX", EAX);
    dictionary_put(copia, "EBX", EBX);
    dictionary_put(copia, "ECX", ECX);
    dictionary_put(copia, "EDX", EDX);

    return copia;
}