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
            //resize_s(proceso, contextoEjecucion->motivoDesalojo->parametros);
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

void resize_s(t_pcb *proceso,char **parametros){

}

// IO_GEN_SLEEP
void io_gen_sleep(t_pcb *proceso, char **parametros)
{
    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;

    loggearBloqueoDeProcesos(proceso, "IO_GEN_SLEEP");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);

    log_info(logger, "PID <%d>-Ejecuta IO_GEN_SLEEP por <%s> unidades de trabajo", proceso->pid, parametros[1]);

    pthread_t pcb_bloqueado;
    if (!pthread_create(&pcb_bloqueado, NULL, dormirIO(proceso, parametros[0], parametros[1]), proceso))
    {
        pthread_detach(pcb_bloqueado);
    }
    else
    {
        log_error(loggerError, "Error al crear hilo para dormir IO");
    }
}



void io_stdin_read(t_pcb *proceso, char **parametros)
{
    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;

    loggearBloqueoDeProcesos(proceso, "IO_STDIN_READ");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);

    log_info(logger, "PID <%d>-Ejecuta IO_STDIN_READ", proceso->pid);

    pthread_t pcb_bloqueado;
    if (!pthread_create(&pcb_bloqueado, NULL, mandar_ejecutar_stdin(proceso, parametros[0], parametros[1], parametros[2]), proceso))
    {
        pthread_detach(pcb_bloqueado);
    }
    else
    {
        log_error(loggerError, "Error al crear hilo para ejecutar IO STDIN");
    }
}

void io_stdout_write(t_pcb *proceso, char **parametros)
{
     estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;

    loggearBloqueoDeProcesos(proceso, "IO_STDOUT_WRITE");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);

    log_info(logger, "PID <%d>-Ejecuta IO_STDOUT_WRITE", proceso->pid);

    pthread_t pcb_bloqueado;
    if (!pthread_create(&pcb_bloqueado, NULL, mandar_ejecutar_stdout(proceso, parametros[0], parametros[1], parametros[2]), proceso))
    {
        pthread_detach(pcb_bloqueado);
    }
    else
    {
        log_error(loggerError, "Error al crear hilo para ejecutar IO STDOUT");
    }
}

void io_fs_create(t_pcb *proceso, char **parametros)
{
}

void io_fs_delete(t_pcb *proceso, char **parametros)
{
}

void io_fs_truncate(t_pcb *proceso, char **parametros)
{
}

void io_fs_write(t_pcb *proceso, char **parametros)
{
}

void io_fs_read(t_pcb *proceso, char **parametros)
{
}


//COMO OBTENGO EL SOCKET CORRESPONDIENTE A ESA INTERFAZ?
//STDOUT
void *mandar_ejecutar_stdout(t_pcb *proceso, char *interfaz,char *registroDireccion, char* registroTamanio){
    //enviarMensajeSTDOUT(socketClienteIO, interfaz, registroDireccion, registroTamanio);//como hago para recuperar el socket del IO que obtuve en escucharIO
        
    // TODO: esperar la rta de IO
    pasarAReady(proceso);
    return; 
}

//STDIN
void *mandar_ejecutar_stdin(t_pcb *proceso, char *interfaz,char *registroDireccion, char* registroTamanio){
    //enviarMensajeSTDIN(socketClienteIO, interfaz, registroDireccion, registroTamanio);//como hago para recuperar el socket del IO que obtuve en escucharIO
        
    // TODO: esperar la rta de IO
    pasarAReady(proceso);
    return; 
}

//GEN SLEEP
void *dormirIO(t_pcb *proceso, char *interfaz, char *tiempo)
{
    
    //enviarMensajeGen(socketClienteIO, interfaz, tiempo);//como hago para recuperar el socket del IO que obtuve en escucharIO
    
    // TODO: esperar la rta de IO
    pasarAReady(proceso);
    return; 
}

// MENSAJES
void enviarMensajeGen(int socket_cliente, char *mensaje, char *entero)
{
    // Asignar memoria para el paquete
    t_paquetebeta *paquete = malloc(sizeof(t_paquete));

    // Asignar el código de operación al paquete
    paquete->codigo_operacion = IO_GEN_SLEEP;

    // Asignar memoria para el buffer
    paquete->buffer = malloc(sizeof(t_buffer));

    // Copiar el mensaje al buffer
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    // Asignar el entero al paquete
    paquete->entero = entero;

    // Calcular el tamaño total del paquete
    int bytes = sizeof(op_code) + sizeof(int) + paquete->buffer->size;

    // Serializar el paquete
    void *a_enviar = malloc(bytes);
    memcpy(a_enviar, &(paquete->codigo_operacion), sizeof(op_code));
    memcpy(a_enviar + sizeof(op_code), &(paquete->entero), sizeof(int));
    memcpy(a_enviar + sizeof(op_code) + sizeof(int), paquete->buffer->stream, paquete->buffer->size);

    // Enviar el paquete a través del socket
    send(socket_cliente, a_enviar, bytes, 0);
    log_info(logger, "La interfaz '%s' dormirá durante %d unidades de tiempo", mensaje, entero);
    // Liberar la memoria asignada
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    free(a_enviar);
}

void enviarMensajeSTDIN(int socketClienteIO, char* nombreInterfaz, char* registroDireccion, char *registroTamanio){
   
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = IO_STDIN_READ;
    paquete->buffer = malloc(sizeof(t_buffer));

    // Calcular el tamaño total del buffer
    int nombre_len = strlen(nombreInterfaz) + 1;
    int param1_len = strlen(registroDireccion) + 1;
    int param2_len = strlen(registroTamanio) + 1;

    int total_size = sizeof(int) + nombre_len + sizeof(int) + param1_len + sizeof(int) + param2_len + sizeof(int);

    paquete->buffer->size = total_size;
    paquete->buffer->stream = malloc(total_size);

    // Serializar los datos en el buffer
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &nombre_len, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(paquete->buffer->stream + desplazamiento, nombreInterfaz, nombre_len);
    desplazamiento += nombre_len;

    memcpy(paquete->buffer->stream + desplazamiento, &param1_len, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(paquete->buffer->stream + desplazamiento, registroDireccion, param1_len);
    desplazamiento += param1_len;

    memcpy(paquete->buffer->stream + desplazamiento, &param2_len, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(paquete->buffer->stream + desplazamiento, registroTamanio, param2_len);
    desplazamiento += param2_len;

    // Serializar y enviar el paquete
    int bytes = paquete->buffer->size + 2 * sizeof(int);
    void *a_enviar = serializarPaquete(paquete, bytes);

    send(socket, a_enviar, bytes, 0);

    free(a_enviar);
    eliminarPaquete(paquete);
}

void enviarMensajeSTDOUT(int socketClienteIO, char* nombreInterfaz, char* registroDireccion, char *registroTamanio){
   
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = IO_STDOUT_WRITE;
    paquete->buffer = malloc(sizeof(t_buffer));

    // Calcular el tamaño total del buffer
    int nombre_len = strlen(nombreInterfaz) + 1;
    int param1_len = strlen(registroDireccion) + 1;
    int param2_len = strlen(registroTamanio) + 1;

    int total_size = sizeof(int) + nombre_len + sizeof(int) + param1_len + sizeof(int) + param2_len + sizeof(int);

    paquete->buffer->size = total_size;
    paquete->buffer->stream = malloc(total_size);

    // Serializar los datos en el buffer
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &nombre_len, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(paquete->buffer->stream + desplazamiento, nombreInterfaz, nombre_len);
    desplazamiento += nombre_len;

    memcpy(paquete->buffer->stream + desplazamiento, &param1_len, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(paquete->buffer->stream + desplazamiento, registroDireccion, param1_len);
    desplazamiento += param1_len;

    memcpy(paquete->buffer->stream + desplazamiento, &param2_len, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(paquete->buffer->stream + desplazamiento, registroTamanio, param2_len);
    desplazamiento += param2_len;

    // Serializar y enviar el paquete
    int bytes = paquete->buffer->size + 2 * sizeof(int);
    void *a_enviar = serializarPaquete(paquete, bytes);

    send(socket, a_enviar, bytes, 0);

    free(a_enviar);
    eliminarPaquete(paquete);
}

//READY
void pasarAReady(t_pcb *proceso)
{
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    ingresarAReady(proceso);
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
    if(algoritmo==VRR){//Si es VRR, encolo el proceso en READYaux
        encolar(pcbsREADYaux,proceso);
    }
}