#include <planificacion/algoritmosCortoPlazo.h>

void detenerYDestruirCronometro(t_temporal *cronometroReady){
    temporal_stop(cronometroReady);
    temporal_destroy(cronometroReady);
}

void planificarACortoPlazoSegunAlgoritmo(){
    log_info(logger, "Planificadior a corto plazo");
    char *algoritmoPlanificador = obtenerAlgoritmoPlanificacion();
    
    if (!strcmp(algoritmoPlanificador, "FIFO"))
    {
        //contextoEjecucion->algoritmo=FIFO;
        //log_info(logger, "Ejecutando FIFO");
        planificarACortoPlazo(proximoAEjecutarFIFO);
    } else if(!strcmp(algoritmoPlanificador, "RR")){
        log_info(logger, "LOGGER DE LA VERDAD");
        //contextoEjecucion->algoritmo=RR;
        log_info(logger, "Ejecutando RR");
        planificarACortoPlazo(proximoAEjecutarRR);
    } else if(!strcmp(algoritmoPlanificador, "VRR")){
        //contextoEjecucion->algoritmo=VRR;
        log_info(logger, "Ejecutando VRR");
        planificarACortoPlazo(proximoAEjecutarVRR);
    } else {
        log_error(loggerError, "Algoritmo invalido");
        abort();
    }

}

t_pcb *proximoAEjecutarFIFO(){
    t_pcb *pcbActual=desencolar(pcbsREADY); //Desencolo el primer pcb de READY
    pcbActual->algoritmo=FIFO;
    return pcbActual;
}

t_pcb *proximoAEjecutarRR(){
    int64_t quantumConfig = obtenerQuantum(); //Obtengo el quantum desde el config

    t_pcb *pcbActual = desencolar(pcbsREADY); //desencolo el primer pcb de READY 
    pcbActual->algoritmo=RR;
    pcbActual->quantum=quantumConfig; 
    //Como el quantum es conocido por el pcb y el contexto de ejcucion lleva la cuenta de las rafagas
    //Puedo delegar la validacion del quantum en el CPU y despues lo manejo en retornoContexto
    
    return pcbActual;    
}

//Van a pcbsREADY:los nuevos y los desalojados por fin de q
//Van a pcbsREADYaux: los que vuelven de IO (q=qConfig-qConsumido)
//La llegada de los procesos a las colas se delega a syscalls
t_pcb *proximoAEjecutarVRR(){
    int64_t quantumConfig = obtenerQuantum();
    if(list_is_empty(pcbsREADYaux)){ //Si esta vacia la auxiliar, desencolo de la principal
        t_pcb *pcbActual = desencolar(pcbsREADY);
        pcbActual->quantum = quantumConfig;
        return pcbActual;
    }
    else{ //Si no, desencolo de la cola auxiliar
        t_pcb *pcbActual = desencolar(pcbsREADYaux);
        int64_t quantumConsumido=temporal_gettime(pcbActual->tiempoDeUsoCPU); //Esto lo obtendria del pcb actualizado
        pcbActual->quantum = quantumConfig-quantumConsumido;
        return pcbActual;
    }
}


