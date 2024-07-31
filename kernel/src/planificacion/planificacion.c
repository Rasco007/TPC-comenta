#include <planificacion/planificacion.h>

sem_t hayProcesosReady;
sem_t hayProcesosNuevos;
pthread_mutex_t pausaMutex;
pthread_cond_t pausaCond;
t_list *pcbsNEW;
t_list *pcbsREADY;
t_list *pcbsREADYaux;
t_list *pcbsExec;
t_list *pcbsBloqueados;
t_list *pcbsParaExit;
int32_t procesosCreados = 0;
pthread_mutex_t mutexListaNew;
pthread_mutex_t mutexListaReady;
pthread_mutex_t mutexListaReadyAux;
pthread_mutex_t mutexListaExec;
pthread_mutex_t mutexListaExit;
pthread_mutex_t mutexListaBloqueados;
pthread_mutex_t mutexPCB;
sem_t semGradoMultiprogramacion;
int64_t rafagaCPU;
bool pausaPlanificacion =false; //Flag para manejar el pausado de la planificacion desde consola
int flag_exit=0;

int gradoMultiprogramacion; 
char *estadosProcesos[5] = {"NEW", "READY", "EXEC", "BLOCKED", "EXIT"}; 
int *instanciasRecursos;

void planificarALargoPlazo(){
    //logger=cambiarNombre(logger,"Kernel-Planificador LP");
    while (1)
    {
        pthread_mutex_lock(&pausaMutex);
        while(pausaPlanificacion){
            pthread_cond_wait(&pausaCond, &pausaMutex);
        }
        pthread_mutex_unlock(&pausaMutex);

       // log_info(logger, "------comienza while largo plazo");
        sem_wait(&hayProcesosNuevos);
        
        int semValue;
        sem_getvalue(&semGradoMultiprogramacion, &semValue);
        log_info(logger,"Valor del semáforo de multiprogramación: %d", semValue);
        sem_wait(&semGradoMultiprogramacion);

        pthread_mutex_lock(&pausaMutex);
        if (pausaPlanificacion) {
            pthread_cond_wait(&pausaCond,&pausaMutex);
        }
        pthread_mutex_unlock(&pausaMutex);

        //log_info(logger, "------obtenerSiguienteAReady");
        pthread_mutex_lock(&mutexPCB);
        t_pcb *pcb = obtenerSiguienteAReady(); //Agarro un pcb de la cola de new
        pthread_mutex_unlock(&mutexPCB);

        //recibirEstructurasInicialesMemoria(pcb); //Mando peticion a memoria

        estadoProceso anterior = pcb->estado;
        pcb->estado = READY; //Lo cambio a estado ready

        loggearCambioDeEstado(pcb->pid, anterior, pcb->estado);
        ingresarAReady(pcb);  
    }
} 


void planificarACortoPlazo(t_pcb *(*proximoAEjecutar)()){
    
    crearColasBloqueo();

    while (1)
    {
        pthread_mutex_lock(&pausaMutex);
        while(pausaPlanificacion){
            pthread_cond_wait(&pausaCond, &pausaMutex);
        }
        pthread_mutex_unlock(&pausaMutex);

        //log_info(logger, "------comienza while corto plazo");
        flag_exit=0;
        //logger=cambiarNombre(logger,"Kernel-Planificador CP");
        sem_wait(&hayProcesosReady);

        pthread_mutex_lock(&pausaMutex);
        if (pausaPlanificacion) {
            pthread_cond_wait(&pausaCond,&pausaMutex);
        }
        pthread_mutex_unlock(&pausaMutex);
         
        t_pcb *aEjecutar = proximoAEjecutar(); //Desencola de Ready segun un algoritmo
        //detenerYDestruirCronometro(aEjecutar->tiempoDeUsoCPU);
        
        pthread_mutex_lock(&mutexListaExec);
        encolar(pcbsExec,aEjecutar);
        pthread_mutex_unlock(&mutexListaExec);
        
        //Paso el proceso a EXEC
        estadoProceso estadoAnterior = aEjecutar->estado;
        aEjecutar->estado = EXEC;

        loggearCambioDeEstado(aEjecutar->pid, estadoAnterior, aEjecutar->estado);

        //Mando el contexto de ejecucion a la CPU por dispatch
        contextoEjecucion = procesarPCB(aEjecutar);
        
        pthread_mutex_lock(&mutexListaExec);
        desencolar(pcbsExec);
        pthread_mutex_unlock(&mutexListaExec);
        
        //Recibo el contexto actualizado
        retornoContexto(aEjecutar, contextoEjecucion);
        //log_info(logger, "APAREZCO KERNEL");
        if(flag_exit==1) continue;
    }
}


// Semaforos

void inicializarSemaforos(){   
    gradoMultiprogramacion = obtenerGradoMultiprogramacion();
    pthread_mutex_init(&mutexListaNew, NULL);
    pthread_mutex_init(&mutexListaReady,NULL); 
    pthread_mutex_init(&mutexListaReadyAux,NULL);
    pthread_mutex_init(&mutexListaExec,NULL);
    pthread_mutex_init(&mutexListaExit,NULL);
    pthread_mutex_init(&mutexListaBloqueados,NULL);
    pthread_mutex_init(&list_mutex,NULL);
    pthread_mutex_init(&mutexPCB,NULL);
    pthread_mutex_init(&mutexProcesoEnEjecucion, NULL);
    sem_init(&hayProcesosNuevos, 0, 0);
    sem_init(&hayProcesosReady, 0, 0);
    sem_init(&semGradoMultiprogramacion, 0, gradoMultiprogramacion);
    sem_init(&memoriaOK,0,0);
}

void destruirSemaforos () {
    pthread_mutex_destroy(&mutexListaNew);
    pthread_mutex_destroy(&mutexListaReady);
    pthread_mutex_destroy(&mutexListaReadyAux);
    pthread_mutex_destroy(&mutexListaExec);
    pthread_mutex_destroy(&mutexListaExit);
    pthread_mutex_destroy(&mutexListaBloqueados);
    pthread_mutex_destroy(&list_mutex);
    pthread_mutex_destroy(&mutexPCB);
    pthread_mutex_destroy(&mutexProcesoEnEjecucion);
    sem_close(&hayProcesosNuevos);
    sem_close(&hayProcesosReady);
    sem_close(&semGradoMultiprogramacion);
    sem_close(&memoriaOK);
    pthread_mutex_init(&pausaMutex, NULL);
    pthread_cond_init(&pausaCond, NULL);
}

//Manejo de colas
void ingresarANew(t_pcb *pcb)
{
    pthread_mutex_lock(&mutexListaNew);
    encolar(pcbsNEW, pcb);
    log_info(logger, "Se crea el proceso <%d> en NEW", pcb->pid);//log obligatorio
    pthread_mutex_unlock(&mutexListaNew);
    sem_post(&hayProcesosNuevos);
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
    
    pthread_mutex_unlock(&mutexListaReady);

    sem_post(&hayProcesosReady);

    pidsInvolucrados = string_new();
    listarPIDS(pcbsREADY);
    log_info(logger, "Cola Ready <%s>: [%s]", obtenerAlgoritmoPlanificacion(), pidsInvolucrados);
    free(pidsInvolucrados);
}

void ingresarAReadyAux(t_pcb *pcb){
    pthread_mutex_lock(&mutexListaReadyAux);
    encolar(pcbsREADYaux, pcb);
    pthread_mutex_unlock(&mutexListaReadyAux);

    sem_post(&hayProcesosReady);

    pidsInvolucrados = string_new();
    listarPIDS(pcbsREADY);
    log_info(logger, "Ready Prioridad <%s>: [%s]", obtenerAlgoritmoPlanificacion(), pidsInvolucrados);
    free(pidsInvolucrados);
}

void loggearCambioDeEstado(uint32_t pid, estadoProceso anterior, estadoProceso actual){//Creo que lo de el estado actual y anterior esta mal. Ver.
    log_info(logger, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", pid, estadosProcesos[anterior], estadosProcesos[actual]);
}

