#include <planificacion/algoritmosCortoPlazo.h>

void detenerYDestruirCronometro(t_temporal *cronometroReady){
    temporal_stop(cronometroReady);
    temporal_destroy(cronometroReady);
}

void planificarACortoPlazoSegunAlgoritmo(){
    log_info(logger, "Planificadior a corto plazo");
    char *algoritmoPlanificador = obtenerAlgoritmoPlanificacion();
     log_info(logger, "algoritmo : %s", algoritmoPlanificador);
    if (!strcmp(algoritmoPlanificador, "FIFO"))
    {
        log_info(logger, "Ejecutando FIFO");
        planificarACortoPlazo(proximoAEjecutarFIFO);
    } else if(!strcmp(algoritmoPlanificador, "RR")){
        log_info(logger, "Ejecutando RR");
        planificarACortoPlazo(proximoAEjecutarRR);
    } else if(!strcmp(algoritmoPlanificador, "VRR")){
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
    
    if(list_is_empty(pcbsREADYaux)){
        t_pcb *pcbActual = desencolar(pcbsREADY);
        pcbActual->quantum = quantumConfig;
        pcbActual->algoritmo=VRR;
        return pcbActual;
    }
    else{
        t_pcb *pcbActual = desencolar(pcbsREADYaux);
        int64_t quantumConsumido=contextoEjecucion->tiempoDeUsoCPU;
        pcbActual->quantum = quantumConfig-quantumConsumido;
        pcbActual->algoritmo=VRR;
        return pcbActual;
    }
}


