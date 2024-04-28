#include <planificacion/algoritmosCortoPlazo.h>

void detenerYDestruirCronometro(t_temporal *cronometroReady){

    temporal_stop(cronometroReady);
    temporal_destroy(cronometroReady);
}

void planificarACortoPlazoSegunAlgoritmo(){
    char *algoritmoPlanificador = obtenerAlgoritmoPlanificacion();

    if (!strcmp(algoritmoPlanificador, "FIFO"))
    {
        planificarACortoPlazo(proximoAEjecutarFIFO);
    }
    else
    {
        log_error(loggerError, "Algoritmo invalido. Debe ingresar FIFO (por ahora)"); //1ro FIFO despues agregar RR y VRR
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

double calcularRR(void *elem){

    t_pcb *pcb = (t_pcb *)elem;

    temporal_stop(pcb->tiempoEnReady);

    double waitTime = temporal_gettime(pcb->tiempoEnReady);

    temporal_resume(pcb->tiempoEnReady);

    double estimatedServiceTime = pcb->estimadoProximaRafaga;

    return (waitTime + estimatedServiceTime) / estimatedServiceTime;
}
