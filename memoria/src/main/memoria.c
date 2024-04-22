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

	usleep (1000 * 500);
	log_info (logger, "Esperando IO...");
	sockets[2] = esperarCliente (server_fd);
	log_info(logger,"Conexion IO OK");
	
	exit (0);
}
