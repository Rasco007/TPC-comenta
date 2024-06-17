#include <escuchaIO/servidorIO.h>
#include <global.h>
#define BUFFER_SIZE 1024

int ejecutarServidorKernel();
void hacerHandshake(int socketClienteIO);
void recibirNombreInterfaz(int socketClienteIO);
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
    recibirNombreInterfaz(*socketClienteIO);

     pthread_create(&thread,
                    NULL,
                    (void*) ejecutarServidorKernel,
                    socketClienteIO);
     pthread_detach(thread);
 }
    
   
    //dormirbeta("Int1", 10, socketClienteIO);
    //log_info(logger,"Conexiones IO-Kernel OK!");
    //ejecutarServidorKernel();
}

void recibirNombreInterfaz(int socketClienteIO){

 char buffer[BUFFER_SIZE] = {0};    

   int valread = recv(socketClienteIO, buffer, BUFFER_SIZE, 0);
    if (valread < 0) {
        perror("recv");
    } else {
        printf("Received: %s\n", buffer);
    }
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
int ejecutarServidorKernel(){
	return 0;
}

