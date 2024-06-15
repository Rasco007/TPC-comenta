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
    int *quantumConfig = obtenerQuantum();

    t_pcb *pcbActual = desencolar(pcbsREADY); 
    pcbActual->quantum=quantumConfig; 
    //Como el quantum es conocido por el pcb y el contexto de ejcucion lleva la cuenta de las rafagas
    //Puedo delegar la validacion del quantum en el CPU y despues lo manejo en retornoContexto
    
    return pcbActual;    
}

//Van a pcbsREADY:los nuevos y los desalojados por fin de q
//Van a pcbsREADYaux: los que vuelven de IO (q=qConfig-qConsumido)
t_pcb *proximoAEjecutarVRR(){
    int *quantumConfig = obtenerQuantum();
    if(list_is_empty(pcbsREADYaux)){
        t_pcb *pcbActual = desencolar(pcbsREADY);
        pcbActual->quantum = quantumConfig;
        return pcbActual;
    }
    else{
        t_pcb *pcbActual = desencolar(pcbsREADYaux);
        int quantumConsumido=pcbActual->rafagasEjecutadas; //Ver
        pcbActual->quantum = quantumConfig-quantumConsumido;
        return pcbActual;
    }
}


