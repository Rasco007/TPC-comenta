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

    enviarContextoActualizado(conexionACPU); //Por dispatch

    if (recibirOperacionDeCPU() < 0) error ("Se desconecto la CPU.");

    recibirContextoActualizado(conexionACPU); 

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
    contextoEjecucion->instruccionesLength = list_size(contextoEjecucion->instrucciones);
    contextoEjecucion->pid = proceso->pid;
    contextoEjecucion->programCounter = proceso->programCounter;
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    contextoEjecucion->registrosCPU = registrosDelCPU(proceso->registrosCPU);
}

t_dictionary *registrosDelCPU(t_dictionary *aCopiar) {
    t_dictionary *copia = dictionary_create();
    char* AX = malloc(sizeof(char) * (4 + 1));
    char* BX = malloc(sizeof(char) * (4 + 1));
    char* CX = malloc(sizeof(char) * (4 + 1));
    char* DX = malloc(sizeof(char) * (4 + 1));
    char* EAX = malloc(sizeof(char) * (8 + 1));
    char* EBX = malloc(sizeof(char) * (8 + 1));
    char* ECX = malloc(sizeof(char) * (8 + 1));
    char* EDX = malloc(sizeof(char) * (8 + 1));
    strncpy(AX, (char *)dictionary_get(aCopiar, "AX"), 4 + 1);
    strncpy(BX, (char *)dictionary_get(aCopiar, "BX"), 4 + 1);
    strncpy(CX, (char *)dictionary_get(aCopiar, "CX"), 4 + 1);
    strncpy(DX, (char *)dictionary_get(aCopiar, "DX"), 4 + 1);
    strncpy(EAX, (char *)dictionary_get(aCopiar, "EAX"), 8 + 1);
    strncpy(EBX, (char *)dictionary_get(aCopiar, "EBX"), 8 + 1);
    strncpy(ECX, (char *)dictionary_get(aCopiar, "ECX"), 8 + 1);
    strncpy(EDX, (char *)dictionary_get(aCopiar, "EDX"), 8 + 1);
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