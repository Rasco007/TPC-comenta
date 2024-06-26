#include <escuchaIO/servidorIO.h>
#include <global.h>
#define BUFFER_SIZE 1024


int ejecutarServidorKernel();
void hacerHandshake(int socketClienteIO);
void recibirNombreInterfaz(int socketClienteIO, Kernel_io *kernel);

void guardarNombreYSocketEnStruct(Kernel_io *kernel, char nombreInterfaz[1024], int socketClienteIO);

void escucharAlIO() {
    char *puertoEscucha = confGet("PUERTO_ESCUCHA");
    int socketKernel = alistarServidorMulti(puertoEscucha);

    while (1) {
        log_info(logger,"Esperando conexiones con IO...");
     pthread_t thread;
    
    int *socketClienteIO = malloc(sizeof(int));
     *socketClienteIO = esperarCliente(socketKernel);
    log_info(logger, "IO conectado, en socket: %d",socketClienteIO);

    hacerHandshake(*socketClienteIO);
    recibirNombreInterfaz(*socketClienteIO, &kernel);

     pthread_create(&thread,
                    NULL,
                    (void*) ejecutarServidorKernel,
                    socketClienteIO);
     pthread_detach(thread);
 }
    
}

void recibirNombreInterfaz(int socketClienteIO, Kernel_io *kernel){

 char nombreInterfaz[BUFFER_SIZE] = {0};    

   int valread = recv(socketClienteIO, nombreInterfaz, BUFFER_SIZE, 0);
    if (valread < 0) {
        log_error(loggerError,"recv");
    } else {
        log_info(logger, "Nombre recibido: %s\n", nombreInterfaz);

        guardarNombreYSocketEnStruct(kernel, nombreInterfaz, socketClienteIO);
    }
}

void guardarNombreYSocketEnStruct(Kernel_io *kernel, char nombreInterfaz[1024], int socketClienteIO)
{
    kernel->interfaces = realloc(kernel->interfaces, (kernel->cantidad + 1) * sizeof(Interfaz));
    if (kernel->interfaces == NULL)
    {
        log_error(loggerError,"Error al redimensionar el arreglo de interfaces");
        exit(EXIT_FAILURE);
    }
    strncpy(kernel->interfaces[kernel->cantidad].nombre_interfaz, nombreInterfaz, sizeof(kernel->interfaces[kernel->cantidad].nombre_interfaz) - 1);
    kernel->interfaces[kernel->cantidad].nombre_interfaz[sizeof(kernel->interfaces[kernel->cantidad].nombre_interfaz) - 1] = '\0';
    kernel->interfaces[kernel->cantidad].socket_interfaz = socketClienteIO;
    kernel->cantidad++;
    log_info(logger,"Se llegó la informacion de la Interfaz %s conectada con socket %d\n", nombreInterfaz, socketClienteIO);

    
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

//esta funcion anda flama
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


int ejecutarServidorKernel(){
     return  0;

}

