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
        log_info(logger, "Ejecutando FIFO");
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


