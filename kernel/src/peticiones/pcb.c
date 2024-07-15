#include <peticiones/pcb.h>
#include "../conexiones/conexionMemoria.h"

char* pidsInvolucrados; 

//Básicos PCB

t_pcb *crearPCB(){
    logger=cambiarNombre(logger,"Kernel-Creacion PCB");
    procesosCreados++;

    t_pcb *nuevoPCB = malloc(sizeof(t_pcb));
    nuevoPCB->estado = NEW;
    nuevoPCB->pid = procesosCreados;
    nuevoPCB->programCounter = 0;
    nuevoPCB->SI = 0;
    nuevoPCB->DI = 0;
    nuevoPCB->instrucciones = list_create();
    nuevoPCB->registrosCPU = crearDiccionarioDeRegistros();
    nuevoPCB->recursosAsignados = list_create();
    nuevoPCB->tablaDePaginas = list_create();
    recibirEstructurasInicialesMemoria(nuevoPCB); //Mando señal a memoria para que reserve espacio para el PCB
    log_info(logger, "PCB con PID %d creado correctamente", nuevoPCB->pid);
    
    return nuevoPCB;
}

void destruirPCB(t_pcb *pcb){
    logger=cambiarNombre(logger,"Kernel-Destruccion PCB");
    int pid_copia = pcb->pid;
    list_add(pcbsParaExit, (void*)(uintptr_t)pid_copia);
    list_destroy_and_destroy_elements(pcb->instrucciones, free);
    dictionary_destroy_and_destroy_elements(pcb->registrosCPU, free);
    log_info(logger, "PCB con PID %d destruido correctamente", pcb->pid);
    free(pcb);
    
}

t_dictionary *crearDiccionarioDeRegistros(){

    t_dictionary *registros = dictionary_create();

    char name[3] = "AX", longName[4] = "EAX";
    for (int i = 0; i < 4; i++) {
        dictionary_put(registros, name, string_repeat('0', 1));
        longName[0] = 'E';
        dictionary_put(registros, longName, string_repeat('0', 4));
        name[0]++, longName[1]++;
    }

    return registros;
}


// Lista de PCBs

void inicializarListasPCBs(){
    pcbsNEW = list_create();
    pcbsREADY = list_create();
    pcbsREADYaux=list_create();
    pcbsEnMemoria = list_create();
    pcbsBloqueados=list_create();
    pcbsParaExit=list_create();
}

void destruirListasPCBs () {
    destruirListaPCB(pcbsNEW);
    destruirListaPCB(pcbsREADY);
    destruirListaPCB(pcbsEnMemoria);
    destruirListaPCB(pcbsBloqueados);
    destruirListaPCB(pcbsREADYaux);
    destruirListaPCB(pcbsParaExit);
}

void destruirListaPCB(t_list *pcbs){
    list_destroy_and_destroy_elements(pcbs, (void *)destruirPCB);
}

void encolar(t_list *pcbs, t_pcb *pcb){
    list_add(pcbs, (void *)pcb);
}

t_pcb *desencolar(t_list *pcbs){
    return (t_pcb *)list_remove(pcbs, 0);
}

void agregarPID(void *value){
    t_pcb *pcb = (t_pcb *)value;
    char *pid = string_itoa(pcb->pid);
    string_append_with_format(&pidsInvolucrados, " %s ", pid);
    free (pid);
}

void listarPIDS(t_list *pcbs) {
    list_iterate(pcbs, agregarPID);
}

void imprimirListaPCBs(t_list *pcbs){
    logger=cambiarNombre(logger,"Kernel-Lista PCBs");
    for(int i = 0; i < list_size(pcbs); i++){
        t_pcb *pcb = list_get(pcbs, i);
        log_info(logger, "PID: %d", pcb->pid);
    }
}

void imprimirListaExit(t_list *idsExit){
    logger=cambiarNombre(logger,"Kernel-Lista Exit");
    for(int i = 0; i < list_size(idsExit); i++){
        log_info(logger, "PID: %d", (int)(uintptr_t)list_get(idsExit, i));
    }
}

t_pcb* buscarPID(t_list* listaPCBs, uint32_t pid){
    
    int cantProcesos = list_size(listaPCBs); 
     
    t_pcb* pcb;
    for(int i=0;i<cantProcesos;i++){
        
        pcb = list_get(listaPCBs, i);
        if(pcb->pid == pid) return pcb;
    }

    return NULL;
}









