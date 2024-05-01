#include <planificacion/algoritmosCortoPlazo.h>

void detenerYDestruirCronometro(t_temporal *cronometroReady){
    temporal_stop(cronometroReady);
    temporal_destroy(cronometroReady);
}

void planificarACortoPlazoSegunAlgoritmo(){
    char *algoritmoPlanificador = obtenerAlgoritmoPlanificacion();
    //Por ahora lo va a hacer por FIFO porque asi esta seteado en el config
    if (!strcmp(algoritmoPlanificador, "FIFO"))
    {
        planificarACortoPlazo(proximoAEjecutarFIFO);
    } else if(!strcmp(algoritmoPlanificador, "RR")){
        //TODO: Implementar RR
    } else if(!strcmp(algoritmoPlanificador, "VRR")){
        //TODO: Implementar VRR
    } else {
        log_error(loggerError, "Algoritmo invalido");
        abort();
    }

}

t_pcb *proximoAEjecutarFIFO(){
    return desencolar(pcbsREADY);
}


void calcularEstimadoProximaRafaga(t_pcb *pcb, int64_t rafagaReal){
    double alfa = obtenerAlfaEstimacion();
    double estimadoRafaga = alfa * rafagaReal + (1 - alfa) * pcb->estimadoProximaRafaga;
    pcb->estimadoProximaRafaga = estimadoRafaga;
}

