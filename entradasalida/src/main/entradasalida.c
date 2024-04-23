#include <main/entradasalida.h>

int main() {

    logger = iniciarLogger("entradasalida.log", "IO");
	loggerError = iniciarLogger("errores.log", "Errores IO");

	config = iniciarConfiguracion("entradasalida.config");

	atexit(terminarPrograma);
	
	//conexionKernel();
	conexionMemoria();

	return EXIT_SUCCESS;
}
