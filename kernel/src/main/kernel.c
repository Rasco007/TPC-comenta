/* - Interprete entre el hardware y el software
- Encargado de iniciar los procesos del sistema 
- Gestiona las peticiones contra la Memoria y las interfaces de I/O
- Planifica la ejecución de los procesos del sistema en el módulo CPU a través de dos conexiones con el mismo: una de dispatch y otra de interrupt.*/

#include <main/kernel.h>
#include "../src/planificacion/planificacion.h"

int socketCliente;
t_log* logger;
t_log* loggerError;
t_config* config;
pthread_t planificadorLargoPlazo_h, planificadorCortoPlazo_h, recibirConsolas_h;

void escucharAlIO();

int main () {
    //Inicializar variables
    logger = iniciarLogger("kernel.log", "Kernel");
	loggerError = iniciarLogger("errores.log", "Errores - Kernel"); 
    config = iniciarConfiguracion("kernel.config");
	atexit (terminarPrograma);

	inicializarSemaforos();
	atexit (destruirSemaforos);
	inicializarListasPCBs(); 
	atexit (destruirListasPCBs);

	conexionMemoria(); 
	conexionCPU();

    char * nombre = string_duplicate("CPU-KERNEL");
	cambiarNombre(logger, nombre);
    escucharAlIO();
	free (nombre);

    //Inicializar Hilos
	int opCodes [2] = {
		pthread_create(&planificadorLargoPlazo_h, NULL, (void *) planificarALargoPlazo, NULL),
		pthread_create(&planificadorCortoPlazo_h, NULL, (void*) planificarACortoPlazoSegunAlgoritmo, NULL)
		//TODO: Agregar hilo para recibir consolas
	};

    if (opCodes [0]) {
        error ("Error al generar hilo para el planificador de largo plazo, terminando el programa.");	
	}
	if (opCodes [1]) {
        error ("Error al generar hilo para el planificador de corto plazo, terminando el programa.");
	}

	t_pcb * primerPCB = crearPCB();
	t_pcb * segundoPCB = crearPCB();
	ingresarAReady(primerPCB);
	ingresarAReady(segundoPCB);
	planificarACortoPlazoSegunAlgoritmo();	
	
	//Esperar a que los hilos terminen
		
	//Hilo Planificador Largo Plazo -> Mueve procesos de NEW a READY
	//pthread_detach(planificadorLargoPlazo_h);
	//Hilo Planificador Corto Plazo --> Mueve procesos de READY a EXEC
	pthread_detach(planificadorCortoPlazo_h);	

    exit (0);
}
