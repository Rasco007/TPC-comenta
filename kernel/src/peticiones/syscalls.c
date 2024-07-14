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
            //existe la interfaz enviada? - hecho
            //esta conectada? - hecho
            //TODO - probarlo en el flujo nomas
            prc_io_gen_sleep(contextoEjecucion, proceso);

            break;
        case IO_STDIN_READ:
            //existe la interfaz enviada?
            //esta conectada?
            //puede ejecutar esta operacion?
            prc_io_stdin_read(contextoEjecucion, proceso);
            break;
        case IO_STDOUT_WRITE:
            //existe la interfaz enviada?
            //esta conectada?
            //puede ejecutar esta operacion?
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
            log_info(logger,"LOGGER DE LA VERDAD");
            finDeQuantum(proceso);
        default:
            log_error(loggerError, "Comando incorrecto");
            break; //falta un case para el FIN_DE_QUANTUM
    }
}

void prc_io_gen_sleep(t_contexto *contextoEjecucion, t_pcb *proceso)
{
    int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);

    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "GENERICA");
        if (esValida == 1)
        {
            // en caso de validar() sea 1, hacemos io_gen_sleep
            ejecutar_io_gen_sleep(proceso, contextoEjecucion->motivoDesalojo->parametros,&kernel);
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "se debe mandar proceso a exit");
        }
    }
    else
    {
        log_info(logger, "se debe mandar proceso a exit");
        // mandar proceso a exit
    }
}

void prc_io_stdin_read(t_contexto *contextoEjecucion, t_pcb *proceso){
     int existeInterfaz = existeLaInterfaz(contextoEjecucion->motivoDesalojo->parametros[0], &kernel);

    if (existeInterfaz == 1)
    {
        int esValida = validarTipoInterfaz(&kernel, contextoEjecucion->motivoDesalojo->parametros[0], "STDIN");
        if (esValida == 1)
        {
            // en caso de validar() sea 1, hacemos io_gen_sleep
            ejecutar_io_stdin_read(proceso, contextoEjecucion->motivoDesalojo->parametros);
        }
        else
        {
            // mandar proceso a exit porque devuelve -1
            log_info(logger, "se debe mandar proceso a exit");
        }
    }
    else
    {
        log_info(logger, "se debe mandar proceso a exit");
        // mandar proceso a exit
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

//IO_GEN_SLEEP [Interfaz, UnidadesDeTrabajo]
void ejecutar_io_gen_sleep(t_pcb *proceso, char **parametros,  Kernel_io *kernel)
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

void* dormirIO(t_pcb * proceso, char* interfaz,char* tiempo){  
     log_info(logger, "tiempo recibido %s", tiempo);
     log_info(logger, "interfaz recibida %s", interfaz);
   int socketClienteIO = obtener_socket(&kernel, interfaz);
   log_info(logger, "se recibio el socket %d", socketClienteIO);
    enviarMensajeGen(socketClienteIO, interfaz, tiempo);
    
     log_info(logger, "antes de recibir msj");
   // Recibir mensaje de confirmacion de IO
    recibirMsjIO( socketClienteIO);
     log_info(logger, "luego e recobor msj");
   // pasarAReady(proceso); SOLO POR AHORA COMENTO ESTO, DESPUES VEMOS BIEN
     
    return NULL; // Add this line to fix the issue
}




void ejecutar_io_stdin_read(t_pcb *proceso, char **parametros)
{
    estadoAnterior = proceso->estado;
    proceso->estado = BLOCKED;

    loggearBloqueoDeProcesos(proceso, "IO_STDIN_READ");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);

    log_info(logger, "PID <%d>-Ejecuta IO_STDIN_READ", proceso->pid);

    pthread_t pcb_bloqueado;
    if (!pthread_create(&pcb_bloqueado, NULL, mandar_ejecutar_stdin(parametros[0], parametros[1], parametros[2]), proceso))
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
    int socketClienteIO = obtener_socket(&kernel, interfaz);
   log_info(logger, "se recibio el socket %d", socketClienteIO);
    enviarMensajeSTDOUT(socketClienteIO, interfaz, registroDireccion, registroTamanio);//como hago para recuperar el socket del IO que obtuve en escucharIO
        
    // TODO: esperar la rta de IO
   // pasarAReady(proceso);
    return NULL; 
}

//STDIN - POR EL MOMENTO LE SAQUE EL PROCESO PARA PROBARLO
void *mandar_ejecutar_stdin(char *interfaz,char *registroDireccion, char* registroTamanio){
   int socketClienteIO = obtener_socket(&kernel, interfaz);
   log_info(logger, "se recibio el socket %d", socketClienteIO);
    
    enviarMensajeSTDIN(socketClienteIO, interfaz, registroDireccion, registroTamanio);
        
    // TODO: esperar la rta de IO
    //pasarAReady(proceso);
    return NULL; 
}

// MENSAJES
/*void enviarMensajeGen(int socket_cliente, char *mensaje, char *entero)
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
}*/

void enviarMensajeSTDIN(int socketClienteIO, char* nombreInterfaz, char* registroDireccion, char *registroTamanio){
    // Crear paquete
    t_paquete *paquete = crearPaquete();

 // Inicializar buffer
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;

    // Agregar la primera variable char* al paquete
    agregarAPaquete(paquete, registroDireccion, strlen(registroDireccion) + 1);

    // Agregar la segunda variable char* al paquete
    agregarAPaquete(paquete, registroTamanio, strlen(registroTamanio) + 1);

    // // Agregar el tamaño al paquete
     //agregarAPaquete(paquete, &tamanio, sizeof(int));

    paquete->codigo_operacion = READ;
     // Asignar memoria para el buffer
   

    // // Agregar la dirección al paquete
     //agregarAPaquete(paquete, direccion, sizeof(void*));

      // Calcular el tamaño total del paquete
   
    
    // Enviar el paquete a través del socket
    enviarPaquete(paquete, socketClienteIO);

    // Eliminar el paquete después de enviarlo
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

    send(socketClienteIO, a_enviar, bytes, 0);

    free(a_enviar);
    eliminarPaquete(paquete);
}



//EXIT
void exit_s(t_pcb *proceso,char **parametros){
    estadoAnterior = proceso->estado;
    proceso->estado = EXIT;
    log_info(logger, "llego al exit");
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    loggearSalidaDeProceso(proceso, parametros[0]);
    
    if(!list_is_empty(proceso->recursosAsignados)){
        liberarRecursosAsignados(proceso);
    }

    liberarMemoriaPCB(proceso);
    list_remove_element(pcbsEnMemoria, proceso);
    destruirPCB(proceso); 
    destroyContextoUnico();
    sem_post(&semGradoMultiprogramacion);
    log_info(logger, "finalizo el exit");
}

//FIN_DE_QUANTUM
void finDeQuantum(t_pcb *proceso){
    t_algoritmo algoritmo=contextoEjecucion->algoritmo;
    
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);

    if(algoritmo==RR){ //Si es RR, encolo el proceso en READY
        ingresarAReady(proceso); //TODO: VER POR QUE TIRA LA OPERACION DESCONOCIDA
    } 
    if(algoritmo==VRR){//Si es VRR, encolo el proceso en READYaux
        //encolar(pcbsREADYaux,proceso); //TODO: Ingresar a auxiliar
    }
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
void pasarAReady(t_pcb *proceso)
{
    estadoAnterior = proceso->estado;
    proceso->estado = READY;
    loggearCambioDeEstado(proceso->pid, estadoAnterior, proceso->estado);
    ingresarAReady(proceso);
}

void recibirMsjIO(int socketClienteIO){
    char buffer[1024];
    int bytes_recibidos = recv(socketClienteIO, buffer, sizeof(buffer), 0);
    
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return ;
    }
    buffer[bytes_recibidos] = '\0'; // Asegurar el carácter nulo al final del mensaje
    
    log_info(logger, "valor recibido: %s", buffer);
}