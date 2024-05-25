
/* - Las interfaces irán recibiendo desde Kernel distintas operaciones a realizar para determinado proceso, 
y le irán dando aviso a dicho módulo una vez completadas.
- Al iniciar una Interfaz de I/O la misma deberá recibir 2 parámetros:
	.Nombre: único dentro del sistema y servirá como identificación de la Interfaz
	.Archivo de Configuración*/

#include "entradasalida.h"

t_log* io_logger = NULL;
t_log* io_loggerError = NULL;
t_config* io_config = NULL;
int fd_kernel = 0;
int fd_memoria = 0;

char* TIPO_INTERFAZ = NULL;
int TIEMPO_UNIDAD_TRABAJO = 0;
char* IP_KERNEL = NULL;
char* PUERTO_KERNEL = NULL;
char* IP_MEMORIA = NULL;
char* PUERTO_MEMORIA = NULL;
char* PATH_BASE_DIALFS = NULL;
int BLOCK_SIZE = 0;
int BLOCK_COUNT = 0;


int main(int argc, char** argv) {


	

    iniciar_io(argc, argv); //abrir modulo con: ./bin/entradasalida /home/utnso/tp-2024-1c-Silver-Crime/entradasalida/entradasalida.config


	//atexit(terminarPrograma);
	
	conexionMemoria();
	//conexionKernel();



	return EXIT_SUCCESS;
}
