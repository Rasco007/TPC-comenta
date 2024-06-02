#include <planificacion/planificacion.h>

sem_t hayProcesosReady;
sem_t hayProcesosNuevos;
t_list *pcbsNEW;
t_list *pcbsREADY;
t_list *pcbsEnMemoria;
int32_t procesosCreados = 0;
pthread_mutex_t mutexListaNew;
pthread_mutex_t mutexListaReady; 
sem_t semGradoMultiprogramacion;
int64_t rafagaCPU;

int gradoMultiprogramacion; 
char *estadosProcesos[5] = {"NEW", "READY", "EXEC", "BLOCKED", "EXIT"}; 
int *instanciasRecursos;

//Faltaria contemplar la elminacion de procesos
void planificarALargoPlazo(){
    while (1)
    {
        sem_wait(&hayProcesosNuevos);

        sem_wait(&semGradoMultiprogramacion);

        t_pcb *pcb = obtenerSiguienteAReady(); //Agarro un pcb de la cola de new

        recibirEstructurasInicialesMemoria(pcb); //Mando peticion a memoria

        estadoProceso anterior = pcb->estado;
        pcb->estado = READY; //Lo cambio a estado ready

        list_add(pcbsEnMemoria, pcb);
        loggearCambioDeEstado(pcb->pid, anterior, pcb->estado);
        ingresarAReady(pcb);   
    }
} 


void planificarACortoPlazo(t_pcb *(*proximoAEjecutar)()){

    crearColasBloqueo();

    while (1)
    {
        sem_wait(&hayProcesosReady);
        t_pcb *aEjecutar = proximoAEjecutar(); //Desencola de Ready segun un algoritmo
        detenerYDestruirCronometro(aEjecutar->tiempoEnReady);
        
        //Paso el proceso a EXEC
        estadoProceso estadoAnterior = aEjecutar->estado;
        aEjecutar->estado = EXEC;

        loggearCambioDeEstado(aEjecutar->pid, estadoAnterior, aEjecutar->estado);

        //Mando el contexto de ejecucion a la CPU por dispatch
        contextoEjecucion = procesarPCB(aEjecutar); 

        rafagaCPU = contextoEjecucion->rafagaCPUEjecutada; 

        //Recibo el contexto actualizado
        retornoContexto(aEjecutar, contextoEjecucion);
    }
}


// Semaforos

void inicializarSemaforos(){   
    gradoMultiprogramacion = obtenerGradoMultiprogramacion();
    pthread_mutex_init(&mutexListaNew, NULL);
    pthread_mutex_init(&mutexListaReady,NULL); 
    sem_init(&hayProcesosNuevos, 0, 0);
    sem_init(&hayProcesosReady, 0, 0);
    sem_init(&semGradoMultiprogramacion, 0, gradoMultiprogramacion);
}

void destruirSemaforos () {
    pthread_mutex_destroy(&mutexListaNew);
    pthread_mutex_destroy(&mutexListaReady);
    sem_close(&hayProcesosNuevos);
    sem_close(&hayProcesosReady);
    sem_close(&semGradoMultiprogramacion);
}

//Manejo de colas
void ingresarANew(t_pcb *pcb)
{
    pthread_mutex_lock(&mutexListaNew);
    encolar(pcbsNEW, pcb);
    log_info(logger, "Se crea el proceso <%d> en NEW", pcb->pid);//log obligatorio
    pthread_mutex_unlock(&mutexListaNew);
}

t_pcb *obtenerSiguienteAReady()
{
    pthread_mutex_lock(&mutexListaNew);
    t_pcb *pcb = desencolar(pcbsNEW);
    pthread_mutex_unlock(&mutexListaNew);
    return pcb;
}

void ingresarAReady(t_pcb *pcb){
    pthread_mutex_lock(&mutexListaReady);
    encolar(pcbsREADY, pcb);
    pcb->tiempoEnReady = temporal_create();
    
    pthread_mutex_unlock(&mutexListaReady);

    sem_post(&hayProcesosReady);

    pidsInvolucrados = string_new();
    listarPIDS(pcbsREADY);
    log_info(logger, "Cola Ready <%s>: [%s]", obtenerAlgoritmoPlanificacion(), pidsInvolucrados);
    free(pidsInvolucrados);
}

void loggearCambioDeEstado(uint32_t pid, estadoProceso anterior, estadoProceso actual){//Creo que lo de el estado actual y anterior esta mal. Ver.
    log_info(logger, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", pid, estadosProcesos[anterior], estadosProcesos[actual]);
}

