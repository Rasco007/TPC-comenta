/* - Las interfaces irán recibiendo desde Kernel distintas operaciones a realizar para determinado proceso, 
y le irán dando aviso a dicho módulo una vez completadas.
- Al iniciar una Interfaz de I/O la misma deberá recibir 2 parámetros:
	.Nombre: único dentro del sistema y servirá como identificación de la Interfaz
	.Archivo de Configuración*/
	
#include <main/entradasalida.h>

int main() {

    logger = iniciarLogger("entradasalida.log", "IO");
	loggerError = iniciarLogger("errores.log", "Errores IO");

	config = iniciarConfiguracion("entradasalida.config");

	atexit(terminarPrograma);
	
	conexionMemoria();
	conexionKernel();


	return EXIT_SUCCESS;
}
