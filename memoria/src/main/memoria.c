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

    int tam_pagina = confGetInt("TAM_PAGINA");
    //enviarMensaje("Mensaje de memoria a cpu",sockets[0]);
    mf = inicializar_memoria_fisica(tam_pagina);
    mf->marcos[8].pid = 1;
    mf->marcos[8].numero_pagina = 6;

    // Inicializa dos procesos con sus archivos de pseudocódigo
    //Espero a que me llege un path
    //sem_t path;
    sem_init(&path, 0, 0);
    sem_wait(&path);
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
    // Ajuste del tamaño del proceso
    //int nuevo_tamano = tam_pagina*4;  // Por ejemplo, ajustar a 3 páginas
    //printf("Número de páginas antes del ajuste: %d\n", proceso->tabla_paginas->paginas_asignadas);
    //proceso = ajustar_tamano_proceso(mf, proceso, nuevo_tamano);
    int nuevo_tamano = tam_pagina*10;  // Por ejemplo, ajustar a 3 páginas : tam_pagina*3
    printf("Número de páginas antes del ajuste: %d\n", proceso->tabla_paginas->paginas_asignadas);
    proceso = ajustar_tamano_proceso(mf, proceso, nuevo_tamano);
    if (proceso == NULL) 
        log_error(loggerError, "Error: No se pudo ajustar el tamaño del proceso.");

    // TODO: el program counter deberia venir del CPU
    int program_counter = 0;
    while (1) {
        char *instruccion1 = obtener_instruccion(proceso, program_counter);

        if (instruccion1) {
            printf("Instrucción del proceso 1: %s", instruccion1);
        }

    // Inicializa dos procesos con sus archivos de pseudocódigo
    //Proceso *proceso1 = inicializar_proceso("src/pseudocodigo/pseucodigo.pc");
    //Proceso *proceso2 = inicializar_proceso("src/pseudocodigo/pseucodigo.pc");

    /*if (!proceso1 || !proceso2) {
        printf("Error al inicializar los procesos.\n");
        liberar_memoria_fisica(mf);
        return 1;
    }

    // Asigna algunas páginas a los procesos
    if (!asignar_pagina(mf, proceso1, 0)) {
        printf("Error al asignar la página 0 al proceso 1.\n");
    }
    if (!asignar_pagina(mf, proceso1, 1)) {
        printf("Error al asignar la página 1 al proceso 1.\n");
    }
    if (!asignar_pagina(mf, proceso2, 0)) {
        printf("Error al asignar la página 0 al proceso 2.\n");
    }

    // TODO: el program counter deberia venir del CPU
    int program_counter = 0;
    while (1) {
        char *instruccion1 = obtener_instruccion(proceso1, program_counter);
        char *instruccion2 = obtener_instruccion(proceso2, program_counter);

        if (instruccion1) {
            printf("Instrucción del proceso 1 en el PC %d: %s", program_counter, instruccion1);
        }
        if (instruccion2) {
            printf("Instrucción del proceso 2 en el PC %d: %s", program_counter, instruccion2);
        }*/

        if (!instruccion1) { //if (!instruccion1 && !instruccion2) {
            break; // Termina si no hay más instrucciones en ambos procesos
        }

        // Simula un retardo en la obtención de la instrucción
        sleep(5);

        program_counter++;
    }

    // Libera memoria
    /*liberar_proceso(proceso1);
    liberar_proceso(proceso2);*/
    liberar_proceso(proceso);
    liberar_memoria_fisica(mf);

    // Espera a que los hilos terminen
    pthread_join(threadCPU, NULL);
    pthread_join(threadIO, NULL);
    pthread_join(threadKernel, NULL);

    exit(0);
}
