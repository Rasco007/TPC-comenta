#include <escuchaIO/servidorIO.h>
#include <global.h>
#define BUFFER_SIZE 1024
int instruccionActual;
char* nombre;
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
        log_info(logger, "IO conectado, en socket: %d",*socketClienteIO);

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
 char tipoInterfaz[BUFFER_SIZE] = {0};  

   int valread = recv(socketClienteIO, nombreInterfaz, BUFFER_SIZE, 0);
    if (valread < 0) {
        log_error(loggerError,"se recibio mal el nombre");
    } else {
        int valreadTipo = recv(socketClienteIO, tipoInterfaz, BUFFER_SIZE, 0);

        if (valreadTipo < 0) {
            log_error(loggerError,"se recibio mal el tipo");
        }else{
            log_info(logger, "Nombre recibido: %s\n", nombreInterfaz);
            log_info(logger, "tipo recibido: %s\n", tipoInterfaz);

            guardarNombreTipoYSocketEnStruct(kernel, nombreInterfaz, tipoInterfaz, socketClienteIO);
            nombre=nombreInterfaz;
        }
        
    }
}

void guardarNombreTipoYSocketEnStruct(Kernel_io *kernel, char nombreInterfaz[256], char tipoInterfaz[256], int socketClienteIO)
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

    // Incrementar la cantidad de interfaces
    kernel->cantidad++;
    
    log_info(logger, "Se han guardado los datos necesarios en la estructura Kernel_io");
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


int ejecutarServidorKernel(int socketClienteIO){
     char * tamaño_max = "1024";
     char * direc_memoria = "1234";
    dormirIO(NULL,nombre,"5");
    mandar_ejecutar_stdin(nombre,direc_memoria, tamaño_max);
    return 0;
     /*while (1) {//ver de solamente poner un recv en vez de while 1 aunque en realidad recibe la operacion en syscalls
        instruccionActual = -1; //habria que importar el ciclo de instrucciones para que la reconozca
		int codOP = recibirOperacion(socketClienteIO);
		switch (codOP) {
			case -1:
				log_info(logger, "El Kernel se desconecto.");
				if (contextoEjecucion != NULL)
					destroyContexto ();
				return EXIT_FAILURE;

			case CONTEXTOEJECUCION:
				return 0;
				break;
			default:
				log_warning(loggerError,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}*/


}


int validarTipoInterfaz(const Kernel_io *kernel, char *nombreInterfaz, char *tipoRequerido){
    for (size_t i = 0; i < kernel->cantidad; i++) {
        log_info(logger, "nombre interfaz: %s", kernel->interfaces[i].nombre_interfaz);
        log_info(logger, "tipo interfaz: %s", kernel->interfaces[i].tipo_interfaz);
        log_info(logger, "tipo requerido: %s", tipoRequerido);
        log_info(logger, "nombre requerido: %s", tipoRequerido);
        if (strcmp(kernel->interfaces[i].nombre_interfaz, nombreInterfaz) == 0) {
            if (strcmp(kernel->interfaces[i].tipo_interfaz, tipoRequerido) == 0) {
                return 1;
            }
        }
    }
    return -1;
}
