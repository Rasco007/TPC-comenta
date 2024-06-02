/* - Obtiene de los archivos de pseudo código las instrucciones y las devuelve a pedido a la CPU.
- Ante cada petición se deberá esperar un tiempo (indicado en el archivo de configuración) determinado 
a modo de retardo en la obtención de la instrucción.
- Trabaja bajo un esquema de paginación simple
- Esta Compuesta por 2 estructuras principales: Un espacio contiguo de memoria y las Tablas de páginas.*/

#include <main/memoria.h>

int sockets[3];
pthread_t threadCPU, threadKernel, threadIO;
Memoria* memoria;

int main() {

    logger = iniciarLogger ("memoria.log", "Memoria");
	loggerError = iniciarLogger ("memoriaErrores.log","Memoria (Errores)"); 
	config = iniciarConfiguracion ("memoria.config");

	atexit (terminarPrograma);

	log_info (logger, "Memoria lista para recibir conexiones.");

	int server_fd = iniciarServidor (confGet("PUERTO_ESCUCHA"));

	sockets[0] = esperarCliente(server_fd);
	log_info(logger, "Memoria conectada a Módulo, en socket: %d", sockets[0]);

	sockets[1] = esperarCliente (server_fd);
	log_info(logger, "Memoria conectada a Módulo, en socket: %d", sockets[1]);

	sockets[2] = esperarCliente (server_fd);
	log_info(logger, "Memoria conectada a Módulo, en socket: %d", sockets[2]);

	// Creación de hilos 
    int opCodeCPU = pthread_create(&threadCPU, NULL, (void*)ejecutarServidorCPU, (void*)&sockets[0]);
    int opCodeIO = pthread_create(&threadIO, NULL, (void*)ejecutarServidorIO, (void*)&sockets[1]);
    int opCodeKernel = pthread_create(&threadKernel, NULL, (void*)ejecutarServidorKernel, (void*)&sockets[2]);

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

	int tamano_pagina = config_get_int_value(config, "TAM_MEMORIA");
    int num_paginas = config_get_int_value(config, "TAM_PAGINA");
    memoria = crearMemoria(num_paginas, tamano_pagina);

    crear_proceso(memoria, 1, 10, "pseudocodigo/procesos/proceso1.pc");

    Pseudocodigo* pseudo = leerPseudocodigo("pseudocodigo/procesos/proceso1.pc");
    char* instruccion = obtenerInstruccion(pseudo, 0);
    if (instruccion != NULL) {
        log_info(logger, "Instrucción obtenida: %s", instruccion);
    }

    destruir_proceso(memoria, 1, 10);

    liberarPseudocodigo(pseudo);

	// Espera a que los hilos terminen
	pthread_join (threadCPU, NULL);
	pthread_join (threadIO, NULL);
	pthread_join (threadKernel, NULL);


	exit(0);
}
