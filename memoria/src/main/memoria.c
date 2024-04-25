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

	// Creación de hilos y logging por separado
    int opCodeCPU = pthread_create(&threadCPU, NULL, (void*)ejecutarServidorCPU, (void*)&sockets[0]);
	log_info(logger, "Memoria conectada a CPU, en socket: %d", sockets[0]);

    int opCodeIO = pthread_create(&threadIO, NULL, (void*)ejecutarServidorIO, (void*)&sockets[1]);
    log_info(logger, "Memoria conectada a IO, en socket: %d", sockets[1]);

    int opCodeKernel = pthread_create(&threadKernel, NULL, (void*)ejecutarServidorKernel, (void*)&sockets[2]);
    log_info(logger, "Memoria conectada a Kernel, en socket: %d", sockets[2]);

    // Verificación de errores en la creación de hilos
    if (opCodeCPU) {
        error("Error en iniciar el servidor a CPU");
    }
    if (opCodeIO) {
        error("Error en iniciar el servidor a IO");
    }
    if (opCodeKernel) {
        error("Error en iniciar el servidor a Kernel");
    }

	// Espera a que los hilos terminen
	pthread_join (threadCPU, NULL);
	pthread_join (threadIO, NULL);
	pthread_join (threadKernel, NULL);


	exit(0);
}
