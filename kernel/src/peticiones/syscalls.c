#include <peticiones/syscalls.h>

t_list *recursos;
char **nombresRecursos;
char* invalidResource = "INVALID_RESOURCE";
char* outOfMemory = "OUT_OF_MEMORY";
estadoProceso estadoAnterior; 
int tiempoIO;
bool hayOpFS;

//FUNCIONES GENERALES
void retornoContexto(t_pcb *proceso, t_contexto *contextoEjecucion){
    //Aca trato las instrucciones bloqueantes
    switch (contextoEjecucion->motivoDesalojo->motivo){
        case WAIT:
            wait_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case RESIZE:
            resize_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case SIGNAL:
            signal_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_GEN_SLEEP:
            io_gen_sleep(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_STDIN_READ:
            io_stdin_read(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_STDOUT_WRITE:
            io_stdout_write(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_CREATE:
            io_fs_create(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_DELETE:
            io_fs_delete(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_TRUNCATE:
            io_fs_truncate(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_WRITE:
            io_fs_write(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_READ:
            io_fs_read(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case EXIT:
            exit_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case FIN_DE_QUANTUM:
            finDeQuantum(proceso);
        default:
            log_error(loggerError, "Comando incorrecto");
            break; //falta un case para el FIN_DE_QUANTUM
    }
}




void loggearBloqueoDeProcesos(t_pcb* proceso, char* motivo) {
    log_info(logger,"PID: <%d> - Bloqueado por: %s", proceso->pid, motivo); //Log obligatorio
}

void loggearSalidaDeProceso(t_pcb* proceso, char* motivo) {
    log_info(logger,"Finaliza el proceso <%d> - Motivo: <%s>", proceso->pid, motivo); //log obligatorio
}

//FUNCIONES RETORNO CONTEXTO
//WAIT [Recurso]
void wait_s(t_pcb *proceso,char **parametros){
    char* recurso=parametros[0];
    int indexRecurso = indiceRecurso(recurso);

    if(indexRecurso==-1){ //Verifico que exista el recurso
        exit_s(proceso,&invalidResource); //Si no existe, va a EXIT
        return;
    }

    //Resto 1 a la instancias del recurso indicado
    int instanciaRecurso=instanciasRecursos[indexRecurso];
    instanciaRecurso--; 
    instanciasRecursos[indexRecurso]=instanciaRecurso;

    log_info(logger,"PID:<%d>-WAIT:<%s>-Instancias:<%d>",proceso->pid,recurso,instanciaRecurso);

    //Si el numero de instancias es menor a 0 el proceso se bloquea
    if(instanciaRecurso<0){
        t_list *colaBloqueadosRecurso=(t_list*)list_get(recursos,indexRecurso);

        estadoAnterior = proceso->estado;
        proceso->estado = BLOCKED;

        list_add(colaBloqueadosRecurso,(void*)proceso);

        loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
        loggearBloqueoDeProcesos(proceso, recurso);
    } else {//Si no, vuelve a cpu
        list_add(proceso->recursosAsignados, (void*)string_duplicate(recurso));
        volverACPU(proceso);
    }
}

void volverACPU(t_pcb* proceso) {
    contextoEjecucion = procesarPCB(proceso);
    rafagaCPU = contextoEjecucion->tiempoDeUsoCPU; 
    retornoContexto(proceso, contextoEjecucion); 
}
//SIGNAL [Recurso]
void signal_s(t_pcb *proceso,char **parametros){
    char *recurso = parametros[0];
    int indexRecurso = indiceRecurso(recurso);

    //Verifico que exista el recurso. Si no existe, va a EXIT
    if (indexRecurso == -1){
        exit_s(proceso, &invalidResource); 
        return;
    }

    //Sumo 1 instancia del recurso que se especifica
    int instancRecurso = instanciasRecursos[indexRecurso];
    instancRecurso++;

    log_info(logger,"PID: <%d> - Signal: <%s> - Instancias: <%d>",proceso->pid, recurso, instancRecurso); 
    eliminarRecursoLista(proceso->recursosAsignados,recurso); 

    instanciasRecursos[indexRecurso]=instancRecurso;

    if(instancRecurso <= 0){
        t_list *colaBloqueadosRecurso = (t_list *)list_get(recursos, indexRecurso);
        t_pcb* pcbDesbloqueado = desencolar(colaBloqueadosRecurso);

        list_add(pcbDesbloqueado->recursosAsignados, (void*)string_duplicate (recurso));

        //estimacionNuevaRafaga(pcbDesbloqueado); 

        estadoAnterior = pcbDesbloqueado->estado;
        pcbDesbloqueado->estado = READY;
        loggearCambioDeEstado(pcbDesbloqueado->pid,estadoAnterior,pcbDesbloqueado->estado); 
        ingresarAReady(pcbDesbloqueado); 
    }
    
    list_add(proceso->recursosAsignados, (void*)string_duplicate(recurso));
    volverACPU(proceso);
    //if (strncmp (parametros[2], "EXIT", 4)) volverACPU(proceso);
}
//RESIZE [Tamanho]
void resize_s(t_pcb *proceso,char **parametros){

}

//IO_GEN_SLEEP [Interfaz, UnidadesDeTrabajo]
void io_gen_sleep(t_pcb *proceso,char **parametros){
    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;

    loggearBloqueoDeProcesos(proceso, "IO_GEN_SLEEP");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    
    log_info(logger,"PID <%d>-Ejecuta IO_GEN_SLEEP por <%s> unidades de trabajo", proceso->pid, parametros[1]);

    pthread_t pcb_bloqueado;
    if(!pthread_create(&pcb_bloqueado, NULL, dormirIO(proceso,parametros[0],parametros[1]), proceso)){
        pthread_detach(pcb_bloqueado);
    } else {
        log_error(loggerError, "Error al crear hilo para dormir IO");
    }
}

void* dormirIO(t_pcb * proceso, char* interfaz,char* tiempo){  
    //enviarMensaje(socketClienteIO, "IO_GEN_SLEEP",tiempo);
    //Le mandaria una señal a la interfaz de IO para que duerma el tiempo que le pase por parametro
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    ingresarAReady(proceso);
    return; // Add this line to fix the issue
}

void io_stdin_read(t_pcb *proceso,char **parametros){

}

void io_stdout_write(t_pcb *proceso,char **parametros){

}

void io_fs_create(t_pcb *proceso,char **parametros){

}

void io_fs_delete(t_pcb *proceso,char **parametros){

}

void io_fs_truncate(t_pcb *proceso,char **parametros){

}

void io_fs_write(t_pcb *proceso,char **parametros){

}

void io_fs_read(t_pcb *proceso,char **parametros){

}

//EXIT
void exit_s(t_pcb *proceso,char **parametros){
    estadoAnterior = proceso->estado;
    proceso->estado = EXIT;

    encolar(pcbsParaExit,proceso);

    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearSalidaDeProceso(proceso, parametros[0]);

    if(!list_is_empty(proceso->recursosAsignados)){
        liberarRecursosAsignados(proceso);
    }

    liberarMemoriaPCB(proceso);
    destroyContextoUnico();
    sem_post(&semGradoMultiprogramacion);
}

//FIN_DE_QUANTUM
void finDeQuantum(t_pcb *proceso){
    t_algoritmo algoritmo=contextoEjecucion->algoritmo;
    
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);

    if(algoritmo==RR){ //Si es RR, encolo el proceso en READY
        encolar(pcbsREADY,proceso);
    } 
    if(algoritmo=VRR){//Si es VRR, encolo el proceso en READYaux
        encolar(pcbsREADYaux,proceso);
    }
}