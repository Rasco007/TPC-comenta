/* - Obtiene de los archivos de pseudo código las instrucciones y las devuelve a pedido a la CPU.
- Ante cada petición se deberá esperar un tiempo (indicado en el archivo de configuración) determinado 
a modo de retardo en la obtención de la instrucción.
- Trabaja bajo un esquema de paginación simple
- Esta Compuesta por 2 estructuras principales: Un espacio contiguo de memoria y las Tablas de páginas.*/

#include <main/memoria.h>

int sockets[3];
pthread_t threadCPU, threadKernel, threadIO;

t_log* logger; 
t_log* loggerError; 
t_config* config; 
MemoriaFisica *mf;
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
    int opCodeIO = pthread_create(&threadIO, NULL, (void*)ejecutarServidorIO, (void*)&sockets[2]);
    int opCodeKernel = pthread_create(&threadKernel, NULL, (void*)ejecutarServidorKernel, (void*)&sockets[1]);

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

    mf= inicializar_memoria_fisica();
    mf->marcos[8].pid = 1;
    mf->marcos[8].numero_pagina = 6;
    // Inicializa dos procesos con sus archivos de pseudocódigo
    //Espero a que me llege un path
    //sem_t path;
    sem_init(&path, 0, 0);
    sem_wait(&path);
    
    //log_info(logger,"path de instrucciones: %s", pathInstrucciones);
   Proceso *proceso = inicializar_proceso(1, pathInstrucciones);

    if (!proceso) {
        printf("Error al inicializar el procesos.\n");
        liberar_memoria_fisica(mf);
        return 1;
    }

    // Asigna algunas páginas a los procesos
    if (!asignar_pagina(mf, proceso, 0)) {
        printf("Error al asignar la página 0 al proceso 1.\n");
    }
    if (!asignar_pagina(mf, proceso, 1)) {
        printf("Error al asignar la página 1 al proceso 1.\n");
    }
    if (!asignar_pagina(mf, proceso, 0)) {
        printf("Error al asignar la página 0 al proceso 2.\n");
    }
    // TODO: el program counter deberia venir del CPU
    int program_counter = 0;
    while (1) {
        char *instruccion1 = obtener_instruccion(proceso, program_counter);

        if (instruccion1) {
            printf("Instrucción del proceso 1: %s", instruccion1);
        }


        if (!instruccion1) {
            break; // Termina si no hay más instrucciones en ambos procesos
        }

        // Simula un retardo en la obtención de la instrucción
        sleep(5);

        program_counter++;
    }

    // Libera memoria
    liberar_proceso(proceso);
    liberar_memoria_fisica(mf);

    // Espera a que los hilos terminen
    pthread_join(threadCPU, NULL);
    pthread_join(threadIO, NULL);
    pthread_join(threadKernel, NULL);

    exit(0);

	
}