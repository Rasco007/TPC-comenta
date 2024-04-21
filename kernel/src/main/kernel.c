/* - Interprete entre el hardware y el software - */
#include <main/kernel.h>


int socketCliente;
t_log* logger;
t_log* loggerError;
t_config* config;
pthread_t planificadorLargoPlazo_h, planificadorCortoPlazo_h, recibirConsolas_h;


int main () {
    //Inicializar variables
    logger = iniciarLogger("kernel.log", "Kernel");
	loggerError = iniciarLogger("errores.log", "Errores - Kernel"); 
    config = iniciarConfiguracion("kernel.config");
	atexit (terminarPrograma);

	conexionIO();
	conexionMemoria(); 
	conexionCPU();
	
    exit (0);
}
