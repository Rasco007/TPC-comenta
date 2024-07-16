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
    
    log_info(logger, "valor recibido: %s", buffer);
}


//FUNCIONES GENERALES
void retornoContexto(t_pcb *proceso, t_contexto *contextoEjecucion){
    logger=cambiarNombre(logger,"Kernel-Retorno Contexto");
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
typedef struct{
    t_pcb* proceso;
    char* tiempo;
    char* interfaz;
}InterfazSalienteGenSleep;

void dormir_IO(InterfazSalienteGenSleep* args){  
    char* interfaz=args->interfaz;
    char* tiempo=args->tiempo;
    t_pcb* proceso=args->proceso;

    log_info(logger, "tiempo recibido %s", tiempo);
    log_info(logger, "interfaz recibida %s", interfaz);
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    log_info(logger, "se recibio el socket %d", socketClienteIO);
    enviarMensajeGen(socketClienteIO, interfaz, tiempo);
    
    log_info(logger, "antes de recibir msj");
    //Recibir mensaje de confirmacion de IO
    recibirMsjIO( socketClienteIO);
    log_info(logger, "luego e recobor msj");
    pasarAReady(proceso);
}

void io_gen_sleep(t_pcb *proceso, char **parametros){
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "GENERICA");
        
        if (esValida == 1){
            // en caso de validar() sea 1, hacemos io_gen_sleep
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_GEN_SLEEP");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_GEN_SLEEP por <%s> unidades de trabajo", proceso->pid, parametros[1]);
            
            InterfazSalienteGenSleep* args = malloc(sizeof(InterfazSalienteGenSleep));
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->tiempo = parametros[1];
            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)dormir_IO, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo para dormir IO");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
    }
    else
    {
        log_info(logger, "no existe la interfaz - proceso a exit");
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
}InterfazSalienteStdinRead;

void ejecutar_io_stdin_read(InterfazSalienteStdinRead* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* direccionFisica=args->direccionFisica;
    char* tamanio=args->tamanio;
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int tamanioInt = atoi(tamanio);
    int direccionFisicaInt = atoi(direccionFisica);
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_STDIN_READ;
    agregarAPaquete(paquete,(void*)&direccionFisicaInt,sizeof(int));
    agregarAPaquete(paquete,(void*)&tamanioInt,sizeof(int));
    enviarPaquete(paquete,socketClienteIO);

    recibirMensaje(socketClienteIO);
    pasarAReady(proceso);
}

void io_stdin_read(t_pcb *proceso,char **parametros){
    InterfazSalienteStdinRead* args=malloc(sizeof(InterfazSalienteStdinRead));

    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDIN");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_STDIN_READ");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_STDIN_READ",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->direccionFisica = parametros[1];
            args->tamanio = parametros[2];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_stdin_read, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
    }
    else
    {
        log_info(logger, "no existe la interfaz - proceso a exit");
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
}InterfazSalienteStdoutWrite;

void ejecutar_io_stdout_write(InterfazSalienteStdoutWrite* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* direccionFisica=args->direccionFisica;
    char* tamanio=args->tamanio;
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    int tamanioInt = atoi(tamanio);
    int direccionFisicaInt = atoi(direccionFisica);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_STDOUT_WRITE;
    agregarAPaquete(paquete,(void*)&direccionFisicaInt,sizeof(int));
    agregarAPaquete(paquete,(void*)&tamanioInt,sizeof(int));
    enviarPaquete(paquete,socketClienteIO);
    recibirMensaje(socketClienteIO);
    pasarAReady(proceso);

}

void io_stdout_write(t_pcb *proceso,char **parametros){
    InterfazSalienteStdoutWrite* args=malloc(sizeof(InterfazSalienteStdoutWrite));

    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDOUT");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_STDOUT_WRITE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_STDOUT_WRITE",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->direccionFisica = parametros[1];
            args->tamanio = parametros[2];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_stdout_write, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
    }
    else
    {
        log_info(logger, "no existe la interfaz - proceso a exit");
        // mandar proceso a exit
        exit_s(proceso,parametros);
    }
}



//IO_FS_CREATE (Interfaz, Nombre Archivo)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
}InterfazSalienteFsCreate;

void ejecutar_io_fs_create(InterfazSalienteFsCreate* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int socketClienteIO = obtener_socket(&kernel, interfaz);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_CREATE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 2*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socketClienteIO, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); 
    }
    recibirMensaje(socketClienteIO);
    pasarAReady(proceso);
}

void io_fs_create(t_pcb *proceso,char **parametros){
    InterfazSalienteFsCreate* args=malloc(sizeof(InterfazSalienteFsCreate));

    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_CREATE",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->nombreArchivo = parametros[1];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_create, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
    }
}
//IO_FS_DELETE (Interfaz, Nombre Archivo)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
}InterfazSalienteFsDelete;

void ejecutar_io_fs_delete(InterfazSalienteFsDelete* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int socketClienteIO = obtener_socket(&kernel, interfaz);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_DELETE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 2*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 

    recibirMensaje(socketClienteIO);
    pasarAReady(proceso);
}

void io_fs_delete(t_pcb *proceso,char **parametros){
    InterfazSalienteFsDelete* args=malloc(sizeof(InterfazSalienteFsDelete));
    
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_DELETE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_DELETE",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->nombreArchivo = parametros[1];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_delete, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
    }
}

//IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño)
typedef struct{
    t_pcb* proceso;
    char* interfaz;
    char* nombreArchivo;
    char* tamanio;
}InterfazSalienteFsTruncate;

void ejecutar_io_fs_truncate(InterfazSalienteFsTruncate* args){
    t_pcb* proceso=args->proceso;
    char* interfaz=args->interfaz;
    char* nombreArchivo=args->nombreArchivo;
    int tamanio=atoi(args->tamanio);
    int socketClienteIO = obtener_socket(&kernel, interfaz);

    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_TRUNCATE;
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 3*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len+sizeof(int), &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 

    recibirMensaje(socketClienteIO);
    pasarAReady(proceso);
}

void io_fs_truncate(t_pcb *proceso,char **parametros){
    InterfazSalienteFsTruncate* args=malloc(sizeof(InterfazSalienteFsTruncate));

    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_TRUNCATE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_TRUNCATE",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->nombreArchivo = parametros[1];
            args->tamanio = parametros[2];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_truncate, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
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
    
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_WRITE;
    /*agregarAPaquete(paquete,(void*)&nombreArchivo,sizeof(char*));
    agregarAPaquete(paquete,(void*)&direccion,sizeof(int));
    agregarAPaquete(paquete,(void*)&tamanio,sizeof(int));
    agregarAPaquete(paquete,(void*)&punteroArchivo,sizeof(char*));
    agregarAPaquete(paquete,(void*)&interfaz,sizeof(char*));*/
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 5*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, &punterito, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 5*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 

    recibirMensaje(socketClienteIO);
    pasarAReady(proceso);
    
}

void io_fs_write(t_pcb *proceso,char **parametros){
    InterfazSalienteFsWrite* args=malloc(sizeof(InterfazSalienteFsWrite));
    
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_WRITE");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_WRITE",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->nombreArchivo = parametros[1];
            args->direccion = parametros[2];
            args->tamanio = parametros[3];
            args->punteroArchivo = parametros[4];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_write, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
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
    
    t_paquete* paquete=crearPaquete();
    paquete->codigo_operacion=IO_FS_READ;
    /*agregarAPaquete(paquete,(void*)&nombreArchivo,sizeof(char*));
    agregarAPaquete(paquete,(void*)&direccion,sizeof(int));
    agregarAPaquete(paquete,(void*)&tamanio,sizeof(int));
    agregarAPaquete(paquete,(void*)&punteroArchivo,sizeof(int));
    agregarAPaquete(paquete,(void*)&interfaz,sizeof(char*));
    enviarPaquete(paquete,socketClienteIO);*/
    paquete->buffer = malloc(sizeof(t_buffer));
    int interfaz_len = strlen(interfaz) ; // +1 para el terminador nulo??????
    int archivo_len = strlen(nombreArchivo);
    paquete->buffer->size = interfaz_len + archivo_len+ 5*sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &interfaz_len, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), interfaz, interfaz_len);
    memcpy(paquete->buffer->stream + sizeof(int)+ interfaz_len, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + 2*sizeof(int)+ interfaz_len, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + 3*sizeof(int)+ interfaz_len, &punterito, sizeof(int));
    memcpy(paquete->buffer->stream + 4*sizeof(int)+ interfaz_len, &archivo_len, sizeof(int));
    memcpy(paquete->buffer->stream + 5*sizeof(int)+ interfaz_len, nombreArchivo, archivo_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    send(socketClienteIO, a_enviar, bytes, 0); 
    recibirMensaje(socketClienteIO);
    pasarAReady(proceso); 
}

void io_fs_read(t_pcb *proceso,char **parametros){
    InterfazSalienteFsRead* args=malloc(sizeof(InterfazSalienteFsRead));

    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);
    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "DialFS");
        
        if (esValida == 1){
            estadoAnterior = proceso->estado;
            proceso->estado = BLOCKED;
            loggearBloqueoDeProcesos(proceso, "IO_FS_READ");
            loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
            log_info(logger, "PID <%d>-Ejecuta IO_FS_READ",proceso->pid);
        
            args->proceso = proceso;
            args->interfaz = parametros[0];
            args->nombreArchivo = parametros[1];
            args->direccion = parametros[2];
            args->tamanio = parametros[3];
            args->punteroArchivo = parametros[4];

            pthread_t pcb_bloqueado;
            if (!pthread_create(&pcb_bloqueado, NULL, (void*)ejecutar_io_fs_read, (void*)args))
            {
                pthread_detach(pcb_bloqueado);
            }
            else
            {
                log_error(loggerError, "Error al crear hilo");
            }        
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "tipo de interfaz invalido - proceso a exit");
            exit_s(proceso,parametros);
        }
    }
}

//EXIT
void exit_s(t_pcb *proceso,char **parametros){
    estadoAnterior = proceso->estado;
    proceso->estado = SALIDA;
    log_info(logger, "llego al exit");
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

void enviarMensajeGen(int socket_cliente, char *mensaje, char *entero_str)
{
    // Convertir el entero de string a int
    int entero = atoi(entero_str);

    // Asignar memoria para el paquete
    t_paquetebeta *paquete = malloc(sizeof(t_paquetebeta));
    if (paquete == NULL) {
        perror("Error al asignar memoria para el paquete");
        return;
    }

    // Asignar el código de operación al paquete
    paquete->codigo_operacion = IO_GEN_SLEEP;

    // Asignar memoria para el buffer
    paquete->buffer = malloc(sizeof(t_buffer));
    if (paquete->buffer == NULL) {
        perror("Error al asignar memoria para el buffer");
        free(paquete);
        return;
    }

    // Copiar el mensaje al buffer
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    if (paquete->buffer->stream == NULL) {
        perror("Error al asignar memoria para el stream del buffer");
        free(paquete->buffer);
        free(paquete);
        return;
    }
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    // Asignar el entero al paquete
    paquete->entero = entero;

    // Calcular el tamaño total del paquete
    int bytes = sizeof(op_code) + sizeof(int) + paquete->buffer->size;

    // Serializar el paquete
    void *a_enviar = malloc(bytes);
    if (a_enviar == NULL) {
        perror("Error al asignar memoria para a_enviar");
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return;
    }
    memcpy(a_enviar, &(paquete->codigo_operacion), sizeof(op_code));
    memcpy(a_enviar + sizeof(op_code), &(paquete->entero), sizeof(int));
    memcpy(a_enviar + sizeof(op_code) + sizeof(int), paquete->buffer->stream, paquete->buffer->size);

    // Log antes de enviar
    log_info(logger, "antes de mandar paquete");

    // Enviar el paquete a través del socket
    int sent_bytes = send(socket_cliente, a_enviar, bytes, 0);
    if (sent_bytes == -1) {
        perror("Error al enviar paquete");
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        free(a_enviar);
        return;
    }

    // Log después de enviar
    log_info(logger, "La interfaz '%s' dormirá durante %s unidades de tiempo", mensaje, entero_str);

    // Liberar la memoria asignada
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
}

//READY
