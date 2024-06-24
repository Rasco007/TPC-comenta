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
//WAIT
void wait_s(t_pcb *proceso,char **parametros){
    char* recurso=parametros[0];
    int indexRecurso = indiceRecurso(recurso);

    if(indexRecurso==-1){
        exit_s(proceso,&invalidResource);
        return;
    }

    int instanciaRecurso=instanciasRecursos[indexRecurso];
    instanciaRecurso--;
    instanciasRecursos[indexRecurso]=instanciaRecurso;

    log_info(logger,"PID:<%d>-WAIT:<%s>-Instancias:<%d>",proceso->pid,recurso,instanciaRecurso);

    if(instanciaRecurso<0){
        t_list *colaBloqueadosRecurso=(t_list*)list_get(recursos,indexRecurso);

        estadoAnterior = proceso->estado;
        proceso->estado = BLOCKED;

        list_add(colaBloqueadosRecurso,(void*)proceso);

        loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
        loggearBloqueoDeProcesos(proceso, recurso);
    } else {
        list_add(proceso->recursosAsignados, (void*)string_duplicate(recurso));
        volverACPU(proceso);
    }
}

void volverACPU(t_pcb* proceso) {
    contextoEjecucion = procesarPCB(proceso);
    rafagaCPU = contextoEjecucion->rafagaCPUEjecutada; 
    retornoContexto(proceso, contextoEjecucion); 
}

void signal_s(t_pcb *proceso,char **parametros){

}

//IO_GEN_SLEEP
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
    char* interfaz=parametros[0];
    char* nombreArchivo = parametros[1];

    log_info(logger, "PID: <%d> - Crear Archivo: <%s>",contextoEjecucion->pid,nombreArchivo);

    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=FCREATE;
    agregarAPaquete(paquete,interfaz,sizeof(interfaz));
    agregarAPaquete(paquete,nombreArchivo,sizeof(nombreArchivo));

    //Falta mandar el paquete a IO para que cree el archivo en FS

    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);
    volverACPU(proceso);

}

void io_fs_delete(t_pcb *proceso,char **parametros){
    char* interfaz=parametros[0];
    char* nombreArchivo = parametros[1];

    log_info(logger, "PID: <%d> - Borrar Archivo: <%s>",contextoEjecucion->pid,nombreArchivo);

    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=FDEL;
    agregarAPaquete(paquete,interfaz,sizeof(interfaz));
    agregarAPaquete(paquete,nombreArchivo,sizeof(nombreArchivo));

    //Falta mandar el paquete a IO para que borre el archivo en FS

    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);
    volverACPU(proceso);
}

void io_fs_truncate(t_pcb *proceso,char **parametros){
    char* interfaz=parametros[0];
    char* nombreArchivo = parametros[1];
    char* tamanioRegistro=parametros[2];

    log_info(logger, "PID: <%d> - Truncar Archivo: <%s>",contextoEjecucion->pid,nombreArchivo);

    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=FTRUNCATE;
    agregarAPaquete(paquete,interfaz,sizeof(interfaz));
    agregarAPaquete(paquete,nombreArchivo,sizeof(nombreArchivo));
    agregarAPaquete(paquete,tamanioRegistro,sizeof(tamanioRegistro));

    //Falta mandar el paquete a IO para que trunque el archivo en FS

    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);
    volverACPU(proceso);
}

void io_fs_write(t_pcb *proceso,char **parametros){
    char* interfaz=parametros[0];
    char* nombreArchivo = parametros[1];
    char* direccionRegistro=parametros[2];
    char* tamanioRegistro=parametros[3];
    char* punteroArchivo=parametros[4];

    log_info(logger, "PID: <%d> - Escribir Archivo: <%s>",contextoEjecucion->pid,nombreArchivo);

    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=FWRITE;
    agregarAPaquete(paquete,interfaz,sizeof(interfaz));
    agregarAPaquete(paquete,nombreArchivo,sizeof(nombreArchivo));
    agregarAPaquete(paquete,direccionRegistro,sizeof(direccionRegistro));
    agregarAPaquete(paquete,tamanioRegistro,sizeof(tamanioRegistro));
    agregarAPaquete(paquete,punteroArchivo,sizeof(punteroArchivo));

    //Falta mandar el paquete a IO para que escriba el archivo en FS

    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);
    volverACPU(proceso);
}

void io_fs_read(t_pcb *proceso,char **parametros){
    char* interfaz=parametros[0];
    char* nombreArchivo = parametros[1];
    char* direccionRegistro=parametros[2];
    char* tamanioRegistro=parametros[3];
    char* punteroArchivo=parametros[4];

    log_info(logger, "PID: <%d> - Leer Archivo: <%s>",contextoEjecucion->pid,nombreArchivo);

    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=FREAD;
    agregarAPaquete(paquete,interfaz,sizeof(interfaz));
    agregarAPaquete(paquete,nombreArchivo,sizeof(nombreArchivo));
    agregarAPaquete(paquete,direccionRegistro,sizeof(direccionRegistro));
    agregarAPaquete(paquete,tamanioRegistro,sizeof(tamanioRegistro));
    agregarAPaquete(paquete,punteroArchivo,sizeof(punteroArchivo));

    //Falta mandar el paquete a IO para que lea el archivo en FS

    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearBloqueoDeProcesos(proceso, nombreArchivo);
    volverACPU(proceso);
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
    char* algoritmo=obtenerAlgoritmoPlanificacion();
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    if(strcmp(algoritmo,"RR")==0){
        encolar(pcbsREADY,proceso);
    } 
    if(strcmp(algoritmo,"VRR")==0){
        encolar(pcbsREADYaux,proceso);
    }
    //ingresarAReady(proceso);?
}