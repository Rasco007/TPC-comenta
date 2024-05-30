
/* - Las interfaces irán recibiendo desde Kernel distintas operaciones a realizar para determinado proceso, 
y le irán dando aviso a dicho módulo una vez completadas.
- Al iniciar una Interfaz de I/O la misma deberá recibir 2 parámetros:
	.Nombre: único dentro del sistema y servirá como identificación de la Interfaz
	.Archivo de Configuración*/

#include <main/entradasalida.h>

t_log* logger;
t_log* loggerError ;
t_config* config ;
int fd_kernel = 0;
int fd_memoria = 0;

char* TIPO_INTERFAZ ;
int TIEMPO_UNIDAD_TRABAJO ;
char* IP_KERNEL;
char* PUERTO_KERNEL ;
char* IP_MEMORIA ;
char* PUERTO_MEMORIA ;
char* PATH_BASE_DIALFS ;
int BLOCK_SIZE ;
int BLOCK_COUNT ;

// yo lo corro con: ./bin/entradasalida IntX entradasalida.config
int main(int argc, char** argv) {

    if (argc < 3) {
    	fprintf(stderr, "Usar: %s IntX </home/utnso/tp-2024-1c-Grupo-SO-/entradasalida/io.config>\n", argv[0]);
    	return EXIT_FAILURE;
	}

    iniciar_io(argv); //abrir modulo con: ./bin/entradasalida /home/utnso/tp-2024-1c-Silver-Crime/entradasalida/entradasalida.config

	atexit(terminarPrograma); //cuando se haga exit, se llamara a terminarPrograma. esto es lo que hace atexit

	conexionMemoria();
	conexionKernel();

	pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) io_atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);
 
	return EXIT_SUCCESS;
	 
}