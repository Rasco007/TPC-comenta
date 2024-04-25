/* - Interprete entre el hardware y el software
- Encargado de iniciar los procesos del sistema 
- Gestiona las peticiones contra la Memoria y las interfaces de I/O
- Planifica la ejecución de los procesos del sistema en el módulo CPU a través de dos conexiones con el mismo: una de dispatch y otra de interrupt.*/

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

	conexionMemoria(); 
	conexionCPU();
	
    exit (0);
}
