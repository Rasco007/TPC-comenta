#include <main/entradasalida.h>

int main() {

    logger = iniciarLogger("entradasalida.log", "IO");
	loggerError = iniciarLogger("errores.log", "Errores IO");
	log_info (logger, "[IO] Logger iniciado correctamente");

	config = iniciarConfiguracion("entradasalida.config");
	log_info (logger, "[IO] Configuracion obtenida correctamente");

	atexit(terminarPrograma);
	
	conexionKernel();
	conexionMemoria();

	return EXIT_SUCCESS;
}
