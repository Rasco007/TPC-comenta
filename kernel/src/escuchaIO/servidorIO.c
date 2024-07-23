#include <escuchaIO/servidorIO.h>
#include <global.h>
#define BUFFER_SIZE 1024
int instruccionActual;
char* nombre;
Interfaz* io_global;

void ejecutarServidorKernel(Interfaz *interfaz_actual);
void hacerHandshake(int socketClienteIO);
void recibirNombreInterfaz(int socketClienteIO, Kernel_io *kernel, Interfaz *interfaz_actual);



void escucharAlIO() {
    io_global= malloc(sizeof(Interfaz));

    char *puertoEscucha = confGet("PUERTO_ESCUCHA");
    int socketKernel = alistarServidorMulti(puertoEscucha);
    //log_info(logger,"Esperando conexiones con IO...");
    while (1) {
        log_info(logger,"Esperando conexiones con IO...");
        
        
        int *socketClienteIO = malloc(sizeof(int));
        *socketClienteIO = esperarCliente(socketKernel);
       // log_info(logger, "IO conectado, en socket: %d",*socketClienteIO);

        hacerHandshake(*socketClienteIO);
        recibirNombreInterfaz(*socketClienteIO, &kernel, io_global);

       
    }
    
}

void recibirNombreInterfaz(int socketClienteIO, Kernel_io *kernel, Interfaz *interfaz_actual){

 char nombreInterfaz[BUFFER_SIZE] = {0};    
 char tipoInterfaz[BUFFER_SIZE] = {0};  

   int valread = recv(socketClienteIO, nombreInterfaz, BUFFER_SIZE, 0);
    if (valread < 0) {
        log_error(loggerError,"se recibio mal el nombre");
    } else {
        int valreadTipo = recv(socketClienteIO, tipoInterfaz, BUFFER_SIZE, 0);

        if (valreadTipo < 0) {
            log_error(loggerError,"se recibio mal el tipo");
        }else{
           // log_info(logger, "Nombre recibido: %s\n", nombreInterfaz);
            //log_info(logger, "tipo recibido: %s\n", tipoInterfaz);

            guardarNombreTipoYSocketEnStruct(kernel, nombreInterfaz, tipoInterfaz, socketClienteIO, interfaz_actual);
            nombre=nombreInterfaz;
        }
        
    }
}

void guardarNombreTipoYSocketEnStruct(Kernel_io *kernel, char nombreInterfaz[256], char tipoInterfaz[256], int socketClienteIO, Interfaz * interfaz_actual)
{
    kernel->interfaces = realloc(kernel->interfaces, (kernel->cantidad + 1) * sizeof(Interfaz));
    if (kernel->interfaces == NULL)
    {
        log_error(loggerError, "Error al redimensionar el arreglo de interfaces");
        exit(EXIT_FAILURE);
    }
    
    // Guardar nombre de la interfaz
    strncpy(kernel->interfaces[kernel->cantidad].nombre_interfaz, nombreInterfaz, sizeof(kernel->interfaces[kernel->cantidad].nombre_interfaz) - 1);
    kernel->interfaces[kernel->cantidad].nombre_interfaz[sizeof(kernel->interfaces[kernel->cantidad].nombre_interfaz) - 1] = '\0';
    
    // Guardar tipo de la interfaz
    strncpy(kernel->interfaces[kernel->cantidad].tipo_interfaz, tipoInterfaz, sizeof(kernel->interfaces[kernel->cantidad].tipo_interfaz) - 1);
    kernel->interfaces[kernel->cantidad].tipo_interfaz[sizeof(kernel->interfaces[kernel->cantidad].tipo_interfaz) - 1] = '\0';
    
    // Guardar socket de la interfaz
    kernel->interfaces[kernel->cantidad].socket_interfaz = socketClienteIO;

    //inicio queue
    kernel->interfaces[kernel->cantidad].cola_procesos_io = queue_create();
    
    //innicio semaforo de bloqueo
     sem_init(&kernel->interfaces[kernel->cantidad].semaforo_cola_procesos, 0, 0);

    *interfaz_actual= kernel->interfaces[kernel->cantidad];
pthread_t thread;
     pthread_create(&thread,
                        NULL,
                        (void*) ejecutarServidorKernel,
                        &kernel->interfaces[kernel->cantidad]);
        pthread_detach(thread);
    // Incrementar la cantidad de interfaces
    kernel->cantidad++;
    
    //log_info(logger, "Se han guardado los datos necesarios en la estructura Kernel_io");
}

void hacerHandshake(int socketClienteIO){
    size_t bytes;

   int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(socketClienteIO, &handshake, sizeof(int32_t), MSG_WAITALL);
   
    if (handshake == 1) {
        bytes = send(socketClienteIO, &resultOk, sizeof(int32_t), 0);
    } else {
        bytes = send(socketClienteIO, &resultError, sizeof(int32_t), 0);
    }
   
}

void inicializarStructsIO(Kernel_io *kernel) {
    kernel->interfaces = NULL;
    kernel->cantidad = 0;
}
void destruirStructsIO (Kernel_io *kernel) {
    free(kernel->interfaces);
    kernel->interfaces = NULL;
    kernel->cantidad = 0;
}

Interfaz *obtener_interfaz(const Kernel_io *kernel, const char *nombre_interfaz) {
    for (size_t i = 0; i < kernel->cantidad; i++) {
        if (strcmp(kernel->interfaces[i].nombre_interfaz, nombre_interfaz) == 0) {
            return &kernel->interfaces[i];
        }
    }
     return NULL;// Si no se encuentra la interfaz
}
int obtener_socket(const Kernel_io *kernel, const char *nombre_interfaz) {
    for (size_t i = 0; i < kernel->cantidad; i++) {
        if (strcmp(kernel->interfaces[i].nombre_interfaz, nombre_interfaz) == 0) {
            return kernel->interfaces[i].socket_interfaz;
        }
    }
    return -1; // Si no se encuentra la interfaz
}
void desconectar_interfaz(Kernel_io *kernel, const char *nombre_interfaz) {
    for (size_t i = 0; i < kernel->cantidad; i++) {
        if (strcmp(kernel->interfaces[i].nombre_interfaz, nombre_interfaz) == 0) {
            kernel->interfaces[i] = kernel->interfaces[kernel->cantidad - 1];
            kernel->cantidad--;
            kernel->interfaces = realloc(kernel->interfaces, kernel->cantidad * sizeof(Interfaz));
            if (kernel->cantidad > 0 && kernel->interfaces == NULL) {
                log_error(loggerError,"Error al redimensionar el arreglo de interfaces");
                exit(EXIT_FAILURE);
            }
            log_info(logger,"Interfaz %s desconectada\n", nombre_interfaz);
            return;
        }
    }
    log_info(logger,"Interfaz %s no encontrada\n", nombre_interfaz);
}

int existeLaInterfaz(char *nombreInterfaz, Kernel_io *kernel){
    //verifico que exista en la estructura
    int socket = obtener_socket(kernel, nombreInterfaz);
    if(socket == -1){
        return -1;
        
    }else{
        int estaConectado = verificarConexionInterfaz(kernel, nombreInterfaz);
        
        return estaConectado;
    }
}

int verificarConexionInterfaz(Kernel_io *kernel, const char *nombre_interfaz) {
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    int max_sd = 0;
    int interfaz_socket = -1;

    for (size_t i = 0; i < kernel->cantidad; i++) {
        if (strcmp(kernel->interfaces[i].nombre_interfaz, nombre_interfaz) == 0) {
            interfaz_socket = kernel->interfaces[i].socket_interfaz;
            break;
        }
    }

    if (interfaz_socket == -1) {
        log_info(logger, "Interfaz %s no encontrada", nombre_interfaz);
        return -1;
    }

    FD_SET(interfaz_socket, &readfds);
    if (interfaz_socket > max_sd) {
        max_sd = interfaz_socket;
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 500000; // 500ms

    int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

    if (activity < 0) {
        perror("select error");
        return -1;
    }

    if (FD_ISSET(interfaz_socket, &readfds)) {
        char buffer[1];
        int valread = recv(interfaz_socket, buffer, sizeof(buffer), MSG_PEEK);

        if (valread == 0) {
            log_info(logger, "Interfaz %s desconectada", nombre_interfaz);
            close(interfaz_socket);
            desconectar_interfaz(kernel, nombre_interfaz);
            return -1;
        }
    }

    return 1;
}


void ejecutarServidorKernel(Interfaz *interfaz_actual){
    //io_global = interfaz_actual;

  pthread_mutex_unlock(&mutex_lista_global);
  list_add(lista_global_io, interfaz_actual);
  pthread_mutex_lock(&mutex_lista_global);
     //log_info(logger, "Puntero io_global: %p", (void*)io_global);
    log_info(logger, "Puntero interfaz?actual: %p", (void*)interfaz_actual);
  
      log_info(logger,"tipo de la innterfaz conectada:  %s", interfaz_actual->tipo_interfaz);
        log_info(logger,"socket de la innterfaz conectada:  %d", interfaz_actual->socket_interfaz); 
    while(true) {
        log_info(logger,"antes de estar en la cola");
        sem_wait(&interfaz_actual->semaforo_cola_procesos);
        t_pcb *pcb;
        pcb = queue_pop(interfaz_actual->cola_procesos_io);
        log_info(logger, "Puntero proceso servidor: %p", (void*)pcb);
        log_info(logger,"hay proceso en la cola");
        ejecutar_io(interfaz_actual, pcb);
        /*if (respuesta_ok) {
            ...
        } else {
            ...
            break
        }*/
    } 
    
     pthread_mutex_unlock(&mutex_lista_global);
    list_remove(lista_global_io, interfaz_actual);
    pthread_mutex_lock(&mutex_lista_global);
   /* wait(mutex_lista_global)
    remove(lista_global, io)
    signal(mutex_lista_global)*/
}
void ejecutar_io(Interfaz *interfaz, t_pcb *proceso){

    log_info(logger,"noombre %s", interfaz->parametro1);
     log_info(logger,"tiempooo %s", interfaz->parametro2);
    if (strcmp(interfaz->tipo_interfaz, "GENERICA") == 0)
    {
        dormir_IO(interfaz, proceso);
    }
    
    
}

//IO_GEN_SLEEP [Interfaz, UnidadesDeTrabajo]
void dormir_IO(Interfaz *interfaz, t_pcb *proceso){  
    //log_warning(logger, "ENTRE A DORMIR IO");
   
    int pid = proceso->pid;
    log_info(logger, "tiempo recibido %s", interfaz->parametro2);
    log_info(logger, "interfaz recibida %s", interfaz->parametro1);
    int socketClienteIO = obtener_socket(&kernel, interfaz);
    log_info(logger, "se recibio el socket %d", socketClienteIO);
    enviarMensajeGen(socketClienteIO, interfaz->parametro1, interfaz->parametro2, pid);
    log_info(logger, "antes de recibir msj");
    //Recibir mensaje de confirmacion de IO
    recibirMsjIO( socketClienteIO);
    log_info(logger, "luego e recobor msj");
    //queue_pop(interfaz->cola_procesos_io);
    //sem_wait(&interfaz->semaforo_cola_procesos);
    //free(args);
    pasarAReady(proceso);
    //free(args);
}

int validarTipoInterfaz(const Kernel_io *kernel, char *nombreInterfaz, char *tipoRequerido){
    for (size_t i = 0; i < kernel->cantidad; i++) {
        /*log_info(logger, "nombre interfaz: %s", kernel->interfaces[i].nombre_interfaz);
        log_info(logger, "tipo interfaz: %s", kernel->interfaces[i].tipo_interfaz);
        log_info(logger, "tipo requerido: %s", tipoRequerido);
        log_info(logger, "nombre requerido: %s", tipoRequerido);*/
        if (strcmp(kernel->interfaces[i].nombre_interfaz, nombreInterfaz) == 0) {
            if (strcmp(kernel->interfaces[i].tipo_interfaz, tipoRequerido) == 0) {
                return 1;
            }
        }
    }
    return -1;
}