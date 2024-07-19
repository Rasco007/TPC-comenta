#include <planificacion/planificacion.h>

sem_t hayProcesosReady;
sem_t hayProcesosNuevos;
pthread_mutex_t pausaMutex;
pthread_cond_t pausaCond;
t_list *pcbsNEW;
t_list *pcbsREADY;
t_list *pcbsREADYaux;
t_list *pcbsEnMemoria;
t_list *pcbsBloqueados;
t_list *pcbsParaExit;
int32_t procesosCreados = 0;
pthread_mutex_t mutexListaNew;
pthread_mutex_t mutexListaReady; 
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

        log_info(logger, "------comienza while largo plazo");
        sem_wait(&hayProcesosNuevos);

        pthread_mutex_lock(&pausaMutex);
        if (pausaPlanificacion) {
            pthread_cond_wait(&pausaCond,&pausaMutex);
        }
        pthread_mutex_unlock(&pausaMutex);

        int semValue;
        sem_getvalue(&semGradoMultiprogramacion, &semValue);
        log_info(logger,"Valor del semáforo de multiprogramación: %d", semValue);
        sem_wait(&semGradoMultiprogramacion);
        //log_info(logger, "------obtenerSiguienteAReady");
        t_pcb *pcb = obtenerSiguienteAReady(); //Agarro un pcb de la cola de new

        //recibirEstructurasInicialesMemoria(pcb); //Mando peticion a memoria

        estadoProceso anterior = pcb->estado;
        pcb->estado = READY; //Lo cambio a estado ready

        //list_add(pcbsEnMemoria, pcb);
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

        log_info(logger, "------comienza while corto plazo");
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
        
        //Paso el proceso a EXEC
        estadoProceso estadoAnterior = aEjecutar->estado;
        aEjecutar->estado = EXEC;

        loggearCambioDeEstado(aEjecutar->pid, estadoAnterior, aEjecutar->estado);

        //Mando el contexto de ejecucion a la CPU por dispatch
        contextoEjecucion = procesarPCB(aEjecutar); 

        //rafagaCPU = contextoEjecucion->tiempoDeUsoCPU; 
       
        //Recibo el contexto actualizado
        retornoContexto(aEjecutar, contextoEjecucion);
        log_info(logger, "APAREZCO KERNEL");
        if(flag_exit==1) continue;
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
    sem_init(&memoriaOK,0,0);
}

void destruirSemaforos () {
    pthread_mutex_destroy(&mutexListaNew);
    pthread_mutex_destroy(&mutexListaReady);
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

void loggearCambioDeEstado(uint32_t pid, estadoProceso anterior, estadoProceso actual){//Creo que lo de el estado actual y anterior esta mal. Ver.
    log_info(logger, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", pid, estadosProcesos[anterior], estadosProcesos[actual]);
}

