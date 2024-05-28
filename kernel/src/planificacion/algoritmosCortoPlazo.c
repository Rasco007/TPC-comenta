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
    char *quantum = confGet("QUANTUM");
    static int index = -1; // Índice para mantener la posición actual en la cola
    static bool primeraVez = true; // Bandera para saber si es la primera iteración

    if (primeraVez || queue_is_empty((t_queue*)pcbsREADY)) {
        // Si es la primera vez o la cola está vacía, reiniciar índice y buscar el primer elemento
        index = 0;
        primeraVez = false;
    } else {
        // Avanzar al siguiente proceso en la cola
        index = (index + 1) % queue_size((t_queue*)pcbsREADY);
    }

    // Obtener el PCB del proceso actual basado en el índice
    t_pcb *pcbActual = list_get(pcbsREADY, index);

    // Verificar si el proceso actual ha agotado su quantum
    if (quantum <= 0) {
        // Si el proceso agotó su quantum, moverlo al final de la cola
        queue_push(queue_pop((t_queue*)pcbsREADY), pcbsREADY);
        // Actualizar el índice para reflejar el cambio
        index = 0;
    }

    // Devolver el proceso seleccionado para ejecución
    return pcbActual;    
}

t_pcb *proximoAEjecutarVRR(){
    //TODO
    return list_get(pcbsREADY, 5);
}


