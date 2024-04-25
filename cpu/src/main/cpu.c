/*- Interpreta y ejecuta las instrucciones de los Contextos de Ejecución recibidos por parte del Kernel. 
Para ello, ejecutará un ciclo de instrucción con los pasos: Fetch, Decode, Execute y Check Interrupt.
- A la hora de ejecutar instrucciones que lo requieran, traducira las direcciones lógicas (propias del proceso) 
a direcciones físicas (propias de la memoria) simulando la existencia de una MMU.
- Durante el transcurso de la ejecución de un proceso, se irá actualizando su Contexto de Ejecución, 
que luego será devuelto al Kernel bajo los siguientes escenarios: finalización del mismo (instrucción EXIT o ante un error), 
solicitar una llamada al Kernel, o deber ser desalojado (interrupción).*/

#include <main/cpu.h>


int main(void){
	
	logger = iniciarLogger("cpu.log", "CPU");
	loggerError = iniciarLogger("errores.log", "Errores CPU");
	
	config = iniciarConfiguracion("cpu.config");

	atexit(terminarPrograma);

	conexionMemoria(); 

	char * nombre = string_duplicate("CPU-KERNEL");
	cambiarNombre(logger, nombre);
    escucharAlKernel();

	free (nombre);
	return EXIT_SUCCESS;
}	