#include <peticiones/syscalls.h>

t_list *recursos;
char **nombresRecursos;
char* invalidResource = "INVALID_RESOURCE";
char* outOfMemory = "OUT_OF_MEMORY";
estadoProceso estadoAnterior; 
int tiempoIO;


void loggearBloqueoDeProcesos(t_pcb* proceso, char* motivo) {
    log_info(logger,"PID: <%d> - Bloqueado por: %s", proceso->pid, motivo); 
}

void loggearSalidaDeProceso(t_pcb* proceso, char* motivo) {
    log_info(logger,"Finaliza el proceso <%d> - Motivo: <%s>", proceso->pid, motivo); 
}

void signal_s(t_pcb *proceso, char **parametros){

    /*char *recurso = parametros[0];

    int indexRecurso = indiceRecurso(recurso);

    if (indexRecurso == -1){
        exit_s(proceso, &invalidResource); 
        return;
    }


    int instancRecurso = instanciasRecursos[indexRecurso];
    instancRecurso++;

    log_info(logger,"PID: <%d> - Signal: <%s> - Instancias: <%d>",proceso->pid, recurso, instancRecurso); 

    eliminarRecursoLista(proceso->recursosAsignados,recurso); 

    instanciasRecursos[indexRecurso]=instancRecurso;

    if(instancRecurso <= 0){
        
        t_list *colaBloqueadosRecurso = (t_list *)list_get(recursos, indexRecurso);

        t_pcb* pcbDesbloqueado = desencolar(colaBloqueadosRecurso);

        list_add(pcbDesbloqueado->recursosAsignados, (void*)string_duplicate (recurso));

        estimacionNuevaRafaga(pcbDesbloqueado); 

        estadoAnterior = pcbDesbloqueado->estado;
        pcbDesbloqueado->estado = READY;

        loggearCambioDeEstado(pcbDesbloqueado->pid,estadoAnterior,pcbDesbloqueado->estado); 
        ingresarAReady(pcbDesbloqueado); 
    
    }
    

    if (strncmp (parametros[2], "EXIT", 4)) volverACPU(proceso);  TODO!  */ 
}

void exit_s(t_pcb* proceso, char **parametros){   
    
    /*estadoAnterior = proceso->estado; 
    proceso->estado = EXIT; 

    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado); 
    loggearSalidaDeProceso(proceso, parametros[0]); 
    
    enviarMensaje("Terminado", proceso->socketPCB);

    if(!list_is_empty(proceso->recursosAsignados)) {
        
        liberarRecursosAsignados(proceso);
    }

    liberarArchivosAsignados(proceso);
    liberarMemoriaPCB(proceso); 

    list_remove_element(pcbsEnMemoria, proceso);
    destruirPCB(proceso); 
    destroyContextoUnico();
    sem_post(&semGradoMultiprogramacion); TODO! */
}

void retornoContexto(t_pcb *proceso, t_contexto *contextoEjecucion){
    log_info(logger, "Comando recibido en: %s", (char*)contextoEjecucion->motivoDesalojo->comando);
    
    switch (contextoEjecucion->motivoDesalojo->comando){
        

        case WAIT:
            wait_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        //TODO: Agregar todos los case
        /*SET, MOV_IN, MOV_OUT, SUM, SUB,
        JNZ, RESIZE, COPY_STRING, WAIT, SIGNAL,
        IO_GEN_SLEEP, IO_STDIN_READ, IO_STDOUT_WRITE, IO_FS_CREATE,
        IO_FS_DELETE, IO_FS_TRUNCATE, IO_FS_WRITE, IO_FS_READ,
        EXIT*/
        /*case SIGNAL:
            signal_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case EXIT:
            exit_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_STDIN_READ:
            fread_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_STDOUT_WRITE:
            fwrite_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_TRUNCATE:
            ftruncate_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_CREATE:
            createSegment_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break;
        case IO_FS_DELETE:
            deleteSegment_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
            break; */
    default:
        log_error(loggerError, "Comando incorrecto, ejecutando Yield para continuar");
        /*yield_s(proceso); TODO!*/
        break;
    }
}

void wait_s(t_pcb *proceso, char **parametros){

    /*char *recurso = parametros[0];

    int indexRecurso = indiceRecurso(recurso);

    if (indexRecurso == -1)
    {
        exit_s(proceso, &invalidResource); 
        return;
    }

    int instancRecurso = instanciasRecursos[indexRecurso];
    instancRecurso--;
    instanciasRecursos[indexRecurso]=instancRecurso;
    

    log_info(logger,"PID: <%d> - Wait: <%s> - Instancias: <%d>",proceso->pid, recurso, instancRecurso); 


    if(instancRecurso < 0){
        
        t_list *colaBloqueadosRecurso = (t_list *)list_get(recursos, indexRecurso);

        estadoAnterior = proceso->estado; 
        proceso->estado = BLOCK;

        list_add(colaBloqueadosRecurso, (void *)proceso);

        loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
        loggearBloqueoDeProcesos(proceso, recurso); 
        
    } 
    else {
        list_add(proceso->recursosAsignados, (void*)string_duplicate (recurso));
       
    
        volverACPU(proceso);
    }*/
}