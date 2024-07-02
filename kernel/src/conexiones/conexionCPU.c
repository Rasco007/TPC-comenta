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

    asignarPCBAContextoBeta(procesoEnEjecucion);
    log_info(logger, "pid %u", contextoEjecucion->pid);
   log_info(logger, "program counter %d", contextoEjecucion->programCounter);
   log_info(logger, "instr lenth %d", contextoEjecucion->instruccionesLength);
    char* primeraInstruccion = list_get(contextoEjecucion->instrucciones, 0);
log_info(logger, "Primera instrucción: %s", primeraInstruccion);

 char* registros[] = {"AX", "BX", "CX", "DX", "EAX", "EBX", "ECX", "EDX"};
    int num_registros = sizeof(registros) / sizeof(registros[0]);

    for (int i = 0; i < num_registros; i++) {
        char* valor = (char*) dictionary_get(contextoEjecucion->registrosCPU, registros[i]);
        log_info(logger, "Registro %s: %s", registros[i], valor);
    }

    enviarContextoBeta(conexionACPU, contextoEjecucion);

    if (recibirOperacionDeCPU() < 0) error ("Se desconecto la CPU.");

    recibirContextoBeta(conexionACPU); 

    actualizarPCB(procesoEnEjecucion);

    free(bufferContexto);
    return contextoEjecucion;
}

void actualizarPCB(t_pcb* proceso){
	list_destroy_and_destroy_elements(proceso->instrucciones, free);
    proceso->instrucciones = list_duplicate(contextoEjecucion->instrucciones);
    proceso->pid = contextoEjecucion->pid;
    proceso->programCounter = contextoEjecucion->programCounter;
	dictionary_destroy_and_destroy_elements(proceso->registrosCPU, free);
    proceso->registrosCPU = registrosDelCPU(contextoEjecucion->registrosCPU);
}

void asignarPCBAContexto(t_pcb* proceso){

    list_destroy_and_destroy_elements(contextoEjecucion->instrucciones, free);
    contextoEjecucion->instrucciones = list_duplicate(proceso->instrucciones);
    //contextoEjecucion->instrucciones = list_create();
    contextoEjecucion->instruccionesLength = 0;
    contextoEjecucion->pid = proceso->pid;
    contextoEjecucion->programCounter = proceso->programCounter;
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    contextoEjecucion->registrosCPU = registrosDelCPU(proceso->registrosCPU);
    contextoEjecucion->tiempoDeUsoCPU=proceso->tiempoDeUsoCPU;
    contextoEjecucion->DI=proceso->DI;
    contextoEjecucion->SI=proceso->SI;
    contextoEjecucion->quantum=proceso->quantum;
    
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
    //contextoEjecucion->instrucciones = list_duplicate(proceso->instrucciones);
    	contextoEjecucion->instrucciones = list_create();
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