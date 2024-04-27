
/* - Las interfaces irán recibiendo desde Kernel distintas operaciones a realizar para determinado proceso, 
y le irán dando aviso a dicho módulo una vez completadas.
- Al iniciar una Interfaz de I/O la misma deberá recibir 2 parámetros:
	.Nombre: único dentro del sistema y servirá como identificación de la Interfaz
	.Archivo de Configuración*/

#include "entradasalida.h"


int main(int argc, char** argv) {


	if (argc < 2) {
			fprintf(stderr, "Usar: %s </home/utnso/tp-2024-1c-Silver-Crime-/entradasalida/entradasalida.config>\n", argv[0]);
			return EXIT_FAILURE;
		}

    iniciar_io(argc, argv); //abrir modulo con: ./bin/entradasalida /home/utnso/tp-2024-1c-Silver-Crime-/entradasalida/entradasalida.config

	atexit(terminarPrograma);
	conexionMemoria();
	conexionKernel();


	return EXIT_SUCCESS;
}
