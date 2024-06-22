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
        contextoEjecucion->algoritmo=FIFO;
        log_info(logger, "Ejecutando FIFO");
        planificarACortoPlazo(proximoAEjecutarFIFO);
    } else if(!strcmp(algoritmoPlanificador, "RR")){
        contextoEjecucion->algoritmo=RR;
        log_info(logger, "Ejecutando RR");
        planificarACortoPlazo(proximoAEjecutarRR);
    } else if(!strcmp(algoritmoPlanificador, "VRR")){
        contextoEjecucion->algoritmo=VRR;
        log_info(logger, "Ejecutando VRR");
        planificarACortoPlazo(proximoAEjecutarVRR);
    } else {
        log_error(loggerError, "Algoritmo invalido");
        abort();
    }

}

t_pcb *proximoAEjecutarFIFO(){
    return desencolar(pcbsREADY);
}

t_pcb *proximoAEjecutarRR(){
    int *quantumConfig = obtenerQuantum(); //Obtengo el quantum desde el config

    t_pcb *pcbActual = desencolar(pcbsREADY); //desencolo el primer pcb de READY 
    pcbActual->quantum=quantumConfig; 
    //Como el quantum es conocido por el pcb y el contexto de ejcucion lleva la cuenta de las rafagas
    //Puedo delegar la validacion del quantum en el CPU y despues lo manejo en retornoContexto
    
    return pcbActual;    
}

//Van a pcbsREADY:los nuevos y los desalojados por fin de q
//Van a pcbsREADYaux: los que vuelven de IO (q=qConfig-qConsumido)
//La llegada de los procesos a las colas se delega a syscalls
t_pcb *proximoAEjecutarVRR(){
    int *quantumConfig = obtenerQuantum();
    if(list_is_empty(pcbsREADYaux)){
        t_pcb *pcbActual = desencolar(pcbsREADY);
        pcbActual->quantum = quantumConfig;
        return pcbActual;
    }
    else{
        t_pcb *pcbActual = desencolar(pcbsREADYaux);
        int quantumConsumido=pcbActual->tiempoDeUsoCPU;
        pcbActual->quantum = quantumConfig-quantumConsumido;
        return pcbActual;
    }
}


