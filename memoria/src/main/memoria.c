#include <main/memoria.h>

int sockets[2];
pthread_t threadCPU, threadKernel;

int main() {

    logger = iniciarLogger ("memoria.log", "Memoria");
	loggerError = iniciarLogger ("memoriaErrores.log","Memoria (Errores)"); 
	config = iniciarConfiguracion ("memoria.config");

	atexit (terminarPrograma);

	int server_fd = iniciarServidor (confGet("PUERTO_ESCUCHA"));

	log_info (logger, "Memoria lista para recibir conexiones.");
	log_info (logger, "Esperando CPU...");
	

	sockets[0] = esperarCliente(server_fd);
	log_info (logger, "Conexion CPU OK");

	usleep (1000 * 500);
	log_info (logger, "Esperando Kernel...");
	sockets[1] = esperarCliente (server_fd);
	log_info(logger,"Conexion KERNEL OK");

	/*
	int opCodes[2] = {
		pthread_create (&threadCPU, NULL, (void*) ejecutarServidorCPU, (void*) &sockets[0]),
		pthread_create (&threadKernel, NULL, (void*) ejecutarServidorKernel, (void*) &sockets[2])
	};

    if (opCodes [0]) {
        error ("Error en iniciar el servidor a CPU");
	}
    if (opCodes [1]) {
        error ("Error en iniciar el servidor a Kernel");
	}
	*/

	/*
	pthread_join (threadCPU, NULL);
	pthread_join (threadKernel, NULL);
	*/

	exit (0);
}
