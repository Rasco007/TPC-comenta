#include <planificacion/algoritmosCortoPlazo.h>

void detenerYDestruirCronometro(t_temporal *cronometroReady){
    temporal_stop(cronometroReady);
    temporal_destroy(cronometroReady);
}

void planificarACortoPlazoSegunAlgoritmo(){
    char *algoritmoPlanificador = obtenerAlgoritmoPlanificacion();
    
    if (!strcmp(algoritmoPlanificador, "FIFO"))
    {
        log_info(logger, "Ejecutando FIFO");
        planificarACortoPlazo(proximoAEjecutarFIFO);
    } else if(!strcmp(algoritmoPlanificador, "RR")){
        log_info(logger, "Ejecutando RR");
        planificarACortoPlazo(proximoAEjecutarRR);
    } else if(!strcmp(algoritmoPlanificador, "VRR")){
        //planificarACortoPlazo(proximoAEjecutarVRR);
    } else {
        log_error(loggerError, "Algoritmo invalido");
        abort();
    }

}

t_pcb *proximoAEjecutarFIFO(){
    return desencolar(pcbsREADY);
}

t_pcb *proximoAEjecutarRR(){
    char *quantum = obtenerQuantum(); //Obtengo el quantum del config

    t_pcb *pcbActual = desencolar(pcbsREADY); //Saco el primer pcb de la cola de ready 

    // Verificar si el proceso actual ha agotado su quantum
    if (quantum <= 0) {
        encolar(pcbsREADY, pcbActual); // Vuelvo a encolar el proceso actual
    }

    // Devolver el proceso seleccionado para ejecución
    return pcbActual;    
}

t_pcb *proximoAEjecutarVRR(){
    /*char *quantum = obtenerQuantum(); //Obtengo el quantum del config
    t_pcb *pcbActual = desencolar(pcbsREADY); // Saco el primer pcb de la cola de ready
    
    if (quantum <= 0) {
        encolar(pcbsREADYaux, pcbActual); //Si termina el quantum lo mando a la cola auxiliar
    } //Es a modo de ejemplo, creo que no era asi...

    return pcbActual;*/
}


