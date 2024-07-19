#include <peticiones/syscalls.h>

t_list *recursos;
char **nombresRecursos;
char* invalidResource = "INVALID_RESOURCE";
estadoProceso estadoAnterior; 

void pasarAReady(t_pcb *proceso)
{
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    if(proceso->algoritmo==VRR){ //Para los procesos que vuelven de hacer IO
        encolar(pcbsREADYaux,proceso);
        pidsInvolucrados = string_new();
        listarPIDS(pcbsREADYaux);
        log_info(logger, "Cola Ready AUX <%s>: [%s]", obtenerAlgoritmoPlanificacion(), pidsInvolucrados);
        free(pidsInvolucrados);
        sem_post(&hayProcesosReady);
    } else ingresarAReady(proceso);
}

void recibirMsjIO(int socketClienteIO){
    char buffer[1024];
    int bytes_recibidos = recv(socketClienteIO, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
    }
    buffer[bytes_recibidos] = '\0'; // Asegurar el carácter nulo al final del mensaje
    //log_info(logger, "valor recibido: %s", buffer);
}

//FUNCIONES GENERALES
void retornoContexto(t_pcb *proceso, t_contexto *contextoEjecucion){
    //logger=cambiarNombre(logger,"Kernel-Retorno Contexto");
    //Aca trato las instrucciones bloqueantes
    switch (contextoEjecucion->motivoDesalojo->motivo){
        case WAIT:
            wait_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
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
            break;
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
        pasarAReady(proceso);
    }
}

//SIGNAL [Recurso]
void signal_s(t_pcb *proceso,char **parametros){
    char *recurso = parametros[0];
    int indexRecurso = indiceRecurso(recurso);

    //Verifico que exista el recurso. Si no existe, va a EXIT
    if (indexRecurso == -1){
        if(!strncmp(parametros[2],"EXIT",4)){
        return;
        } else{
            exit_s(proceso, &invalidResource); 
            return;
        } 
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

        estadoAnterior = pcbDesbloqueado->estado;
        pcbDesbloqueado->estado = READY;
        loggearCambioDeEstado(pcbDesbloqueado->pid,estadoAnterior,pcbDesbloqueado->estado); 
        pasarAReady(pcbDesbloqueado); 
    }
    
    //Si invoco signal para liberar los recursos, termino la funcion. Si no, paso el proceso a ready
    if(!strncmp(parametros[2],"EXIT",4)){
        return;
    } else{
        list_add(proceso->recursosAsignados, (void*)string_duplicate(recurso));
        pasarAReady(proceso);
    }
}

//IO_GEN_SLEEP [Interfaz, UnidadesDeTrabajo]
void dormir_IO(InterfazSalienteGenSleep* args){  
    //log_warning(logger, "ENTRE A DORMIR IO");
    char* interfaz=args->interfaz;
    char* tiempo=args->tiempo;
    t_pcb* proceso=args->proceso;
    int pid = proceso->pid;
    log_info(logger, "tiempo recibido %s", tiempo);
    log_info(logger, "interfaz recibida %s", interfaz);
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    log_info(logger, "se recibio el socket %d", socketClienteIO);
    enviarMensajeGen(socketClienteIO, interfaz, tiempo, pid);
    //log_info(logger, "antes de recibir msj");
    //Recibir mensaje de confirmacion de IO
    recibirMsjIO( socketClienteIO);
    //log_info(logger, "luego e recobor msj");
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);
}

//     INICIAR_PROCESO /home/utnso/tp-2024-1c-Silver-Crime/memoria/src/scripts_memoria/PLANI_1
void io_gen_sleep(t_pcb *proceso, char **parametros){
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    InterfazSalienteGenSleep* args = malloc(sizeof(InterfazSalienteGenSleep));
    args->colaBloqueados = queue_create();
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "GENERICA");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            log_warning(logger, "Proceso <%d> encolado en cola de bloqueados", proceso->pid);
            // en caso de validar() sea 1, hacemos io_gen_sleep
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_GEN_SLEEP");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_GEN_SLEEP por <%s> unidades de trabajo", proceso->pid, parametros[1]);
            // si la cola de bloqueados esta vacia, ejecutar
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->tiempo = parametros[1];
                pthread_t pcb_bloqueado;
                //log_warning(logger, "Proceso: <%d> - Interfaz: <%s> - Tiempo: <%s>",args->proceso->pid, args->interfaz, args->tiempo);
                /*if (!pthread_create(&pcb_bloqueado, NULL, (void*)dormir_IO, (void *) args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo para dormir IO");*/
                pthread_create(&pcb_bloqueado, NULL, (void*)dormir_IO, (void *) args);
                pthread_join(pcb_bloqueado,NULL);
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* direccionFisica;
    char* tamanio;
    t_queue* colaBloqueados;
}InterfazSalienteStdinRead;

void ejecutar_io_stdin_read(InterfazSalienteStdinRead* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* direccionFisica=args->direccionFisica;
    char* tamanio=args->tamanio;
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int tamanioInt = atoi(tamanio);
    int direccionFisicaInt = atoi(direccionFisica);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_STDIN_READ;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &direccionFisicaInt, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &tamanioInt, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int), &pid, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);
}

void io_stdin_read(t_pcb *proceso,char **parametros){
    InterfazSalienteStdinRead* args=malloc(sizeof(InterfazSalienteStdinRead));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDIN");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_STDIN_READ");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_STDIN_READ",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->direccionFisica = parametros[1];
                args->tamanio = parametros[2];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_stdin_read, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* direccionFisica;
    char* tamanio;
    t_queue* colaBloqueados;
}InterfazSalienteStdoutWrite;

void ejecutar_io_stdout_write(InterfazSalienteStdoutWrite* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* direccionFisica=args->direccionFisica;
    char* tamanio=args->tamanio;
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int tamanioInt = atoi(tamanio);
    int direccionFisicaInt = atoi(direccionFisica);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_STDOUT_WRITE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &direccionFisicaInt, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &tamanioInt, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int), &pid, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);

}

void io_stdout_write(t_pcb *proceso,char **parametros){
    InterfazSalienteStdoutWrite* args=malloc(sizeof(InterfazSalienteStdoutWrite));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDOUT");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_STDOUT_WRITE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_STDOUT_WRITE",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->direccionFisica = parametros[1];
                args->tamanio = parametros[2];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_stdout_write, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//IO_FS_CREATE (Interfaz, Nombre Archivo)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
    t_queue* colaBloqueados;
}InterfazSalienteFsCreate;

void ejecutar_io_fs_create(InterfazSalienteFsCreate* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_CREATE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);
}

void io_fs_create(t_pcb *proceso,char **parametros){
    InterfazSalienteFsCreate* args=malloc(sizeof(InterfazSalienteFsCreate));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_CREATE",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->nombreArchivo = parametros[1];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_create, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}
//IO_FS_DELETE (Interfaz, Nombre Archivo)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
    t_queue* colaBloqueados;
}InterfazSalienteFsDelete;

void ejecutar_io_fs_delete(InterfazSalienteFsDelete* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_DELETE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &pid, sizeof(int)); 
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);
}

void io_fs_delete(t_pcb *proceso,char **parametros){
    InterfazSalienteFsDelete* args=malloc(sizeof(InterfazSalienteFsDelete));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_DELETE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_DELETE",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->nombreArchivo = parametros[1];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_delete, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
    char* tamanio;
    t_queue* colaBloqueados;
}InterfazSalienteFsTruncate;

void ejecutar_io_fs_truncate(InterfazSalienteFsTruncate* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int tamanio=atoi(args->tamanio);
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_TRUNCATE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 4*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);
}

void io_fs_truncate(t_pcb *proceso,char **parametros){
    InterfazSalienteFsTruncate* args=malloc(sizeof(InterfazSalienteFsTruncate));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_TRUNCATE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_TRUNCATE",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->nombreArchivo = parametros[1];
                args->tamanio = parametros[2];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_truncate, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
    char* direccion; //logica o fisica?
    char* tamanio;
    char* punteroArchivo;
    t_queue* colaBloqueados;
}InterfazSalienteFsWrite;

void ejecutar_io_fs_write(InterfazSalienteFsWrite* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int direccion=atoi(args->direccion);
    int tamanio=atoi(args->tamanio);
    char* punteroArchivo=args->punteroArchivo;
    int punterito = atoi(punteroArchivo);
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_WRITE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 6*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, &punterito, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 5*sizeof(int)+ interfaz_len, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 6*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso);
    
}

void io_fs_write(t_pcb *proceso,char **parametros){
    InterfazSalienteFsWrite* args=malloc(sizeof(InterfazSalienteFsWrite));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_WRITE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_WRITE",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->nombreArchivo = parametros[1];
                args->direccion = parametros[2];
                args->tamanio = parametros[3];
                args->punteroArchivo = parametros[4];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_write, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
    char* direccion; //logica o fisica?
    char* tamanio;
    char* punteroArchivo;
    t_queue* colaBloqueados;
}InterfazSalienteFsRead;

void ejecutar_io_fs_read(InterfazSalienteFsRead* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int direccion=atoi(args->direccion);
    int tamanio=atoi(args->tamanio);
    char* punteroArchivo=args->punteroArchivo;
    int punterito = atoi(punteroArchivo);
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int pid = proceso->pid;
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_READ;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 6*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, &punterito, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 5*sizeof(int)+ interfaz_len, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 6*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    recibirMensaje(socketClienteIO);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
    queue_pop(args->colaBloqueados);
    free(args);
    pasarAReady(proceso); 
}

void io_fs_read(t_pcb *proceso,char **parametros){
    InterfazSalienteFsRead* args=malloc(sizeof(InterfazSalienteFsRead));
    args->colaBloqueados = queue_create();
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1){
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        if (esValida == 1){
            queue_push(args->colaBloqueados, &proceso->pid);
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_READ");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_READ",proceso->pid);
            int pidBloqueado = *(int*)queue_peek(args->colaBloqueados);
            if (pidBloqueado == proceso->pid){
                args->proceso = proceso;
                args->interfaz = parametros[0];
                args->nombreArchivo = parametros[1];
                args->direccion = parametros[2];
                args->tamanio = parametros[3];
                args->punteroArchivo = parametros[4];
                pthread_t pcb_bloqueado;
                if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_read, (void*)args))
                    pthread_join(pcb_bloqueado,NULL);
                else
                    log_error(loggerError, "Error al crear hilo");
            }
        }
        else{
            // mandar proceso a exit porque devuelve -1
            log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
            exit_s(proceso,parametros);
        }
    }
    else{
        log_warning(logger, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid);
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}

//EXIT
void exit_s(t_pcb *proceso,char **parametros){
    estadoAnterior = proceso->estado;
    proceso->estado = SALIDA;
    //log_info(logger, "llego al exit");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearSalidaDeProceso(proceso, parametros[0]);
    
    if(!list_is_empty(proceso->recursosAsignados)){
        liberarRecursosAsignados(proceso);
    }

    flag_exit=1;
    liberarMemoriaPCB(proceso);
    list_remove_element(pcbsEnMemoria, proceso);
    destruirPCB(proceso); 
    destroyContextoUnico();
    sem_post(&semGradoMultiprogramacion);
    log_info(logger, "finalizo el exit");
    //TODO: ver de encolar en pcbsParaExit
}

//FIN_DE_QUANTUM
void finDeQuantum(t_pcb *proceso){
    log_info(logger,"PID: <%d> - Desalojado por fin de Quantum",proceso->pid); //Log obligatorio
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    //No importa si es RR o VRR, siempre se encola en READY
    ingresarAReady(proceso); 
}

void enviarMensajeGen(int socket_cliente, char *mensaje, char *entero_str, int pid){
    // Convertir el entero de string a int
    int entero = atoi(entero_str);
    t_paquete* paquete=crearPaquete();
    int length = strlen(mensaje);
    paquete->codigo_operacion=IO_GEN_SLEEP;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 2*sizeof(int) + length;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &entero, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &pid, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int), mensaje, length);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket_cliente, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
}