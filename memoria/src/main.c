#include <stdlib.h>
#include <stdio.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <includes/utils/utils.h>
#include "../../utils/src/connections/connection.h"

int sockets[3];
pthread_t threadCPU, threadFS, threadKernel;

int main(int argc, char* argv[]){
    t_log* memoriaLogger = iniciarMemoriaLogger();
    if (memoriaLogger == NULL){
        return EXIT_FAILURE;
    }

    tMemoriaConfig* memoriaConfig = leerMemoriaConfig(memoriaLogger);
    if (memoriaConfig == NULL){
        liberarLogger(memoriaLogger);
        return EXIT_FAILURE;
    }
    
    log_info(memoriaLogger, "¡Saludos desde la Memoria!");
	
	int server_fd = iniciarServidor(memoriaLogger, memoriaConfig->puertoEscucha, 35880);

	log_info(memoriaLogger, "Conectando CPU");
	sockets[0] = esperarCliente(memoriaLogger, "CPU", sockets[0]);
	usleep(1000 * memoriaConfig->retardoRespuesta);

	log_info(memoriaLogger, "Conectando IO");
    sockets[1] = esperarCliente(memoriaLogger, "IO", sockets[1]);
	usleep(1000 * memoriaConfig->retardoRespuesta);

	log_info(memoriaLogger, "Conectando Kernel");
	sockets[2] = esperarCliente(memoriaLogger, "Kernel", sockets[2]);

	int opCodes[3] = {
		pthread_create(&threadCPU, NULL,(void*) crearConexion, (void*) &sockets[0]),
		pthread_create(&threadFS, NULL,(void*) crearConexion, (void*) &sockets[1]),
		pthread_create(&threadKernel, NULL,(void*) crearConexion, (void*) &sockets[2])
	};

    if(opCodes [0]){
        log_error(memoriaLogger,"Error en iniciar el servidor a CPU");
	}
    if(opCodes [1]){
        log_error(memoriaLogger,"Error en iniciar el servidor a FS");
	}
    if(opCodes [2]){
        log_error(memoriaLogger,"Error en iniciar el servidor a Kernel");
	}

	pthread_join(threadCPU, NULL);
	pthread_join(threadFS, NULL);
	pthread_join(threadKernel, NULL);

	exit(0);

    return 0;
}
