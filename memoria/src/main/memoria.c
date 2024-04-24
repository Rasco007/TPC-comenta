#include <main/memoria.h>

int sockets[3];
pthread_t threadCPU, threadKernel, threadIO;

int main() {

    logger = iniciarLogger ("memoria.log", "Memoria");
	loggerError = iniciarLogger ("memoriaErrores.log","Memoria (Errores)"); 
	config = iniciarConfiguracion ("memoria.config");

	atexit (terminarPrograma);

	log_info (logger, "Memoria lista para recibir conexiones.");

	int server_fd = iniciarServidor (confGet("PUERTO_ESCUCHA"));

	sockets[0] = esperarCliente(server_fd);
	
	usleep (1000 * 500);

	sockets[1] = esperarCliente (server_fd);

	usleep (1000 * 500);
	
	sockets[2] = esperarCliente (server_fd);

	int opCodes[3] = {
		pthread_create (&threadCPU, NULL, (void*) ejecutarServidorCPU, (void*) &sockets[0]),
		pthread_create (&threadIO, NULL, (void*) ejecutarServidorIO, (void*) &sockets[1]), 
		pthread_create (&threadKernel, NULL, (void*) ejecutarServidorKernel, (void*) &sockets[2])
	};

    if (opCodes [0]) {
        error ("Error en iniciar el servidor a CPU");
	} 
    if (opCodes [1]) {
        error ("Error en iniciar el servidor a IO");
	}
    if (opCodes [2]) {
        error ("Error en iniciar el servidor a Kernel");
	} 

	pthread_join (threadCPU, NULL);
	pthread_join (threadIO, NULL);
	pthread_join (threadKernel, NULL);


	exit(0);
}
