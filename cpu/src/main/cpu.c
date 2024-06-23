/*- Interpreta y ejecuta las instrucciones de los Contextos de Ejecución recibidos por parte del Kernel. 
Para ello, ejecutará un ciclo de instrucción con los pasos: Fetch, Decode, Execute y Check Interrupt.
- A la hora de ejecutar instrucciones que lo requieran, traducira las direcciones lógicas (propias del proceso) 
a direcciones físicas (propias de la memoria) simulando la existencia de una MMU.
- Durante el transcurso de la ejecución de un proceso, se irá actualizando su Contexto de Ejecución, 
que luego será devuelto al Kernel bajo los siguientes escenarios: finalización del mismo (instrucción EXIT o ante un error), 
solicitar una llamada al Kernel, o deber ser desalojado (interrupción).*/

#include <main/cpu.h>

TLB tlb;
uint64_t tiempo_actual = 0; // Contador de tiempo para LRU


int main(void){
	
	logger = iniciarLogger("cpu.log", "CPU");
	loggerError = iniciarLogger("errores.log", "Errores CPU");

	config = iniciarConfiguracion("cpu.config");

	atexit(terminarPrograma);

	conexionMemoria(); 

	char * nombre = string_duplicate("CPU-KERNEL");
	cambiarNombre(logger, nombre);

    char *algoritmoTLB = obtenerAlgoritmoTLB();
	inicializar_tlb(algoritmoTLB); 

	

    escucharAlKernel();

	// Insertar algunas entradas de ejemplo en la TLB
    /*tlb.entries[0] = (TLBEntry){.pid = 1, .page_number = 7, .frame_number = 100, .valid = true, .last_used = tiempo_actual++};
    tlb.entries[1] = (TLBEntry){.pid = 2, .page_number = 3, .frame_number = 101, .valid = true, .last_used = tiempo_actual++};
    tlb.entries[2] = (TLBEntry){.pid = 1, .page_number = 4, .frame_number = 102, .valid = true, .last_used = tiempo_actual++};
    tlb.entries[3] = (TLBEntry){.pid = 3, .page_number = 8, .frame_number = 103, .valid = true, .last_used = tiempo_actual++};*/
    
    // Consultar una dirección lógica
    /*char* direccionLogica = "12345";
    uint32_t dirFisica = mmu(1, direccionLogica, 0);
    
    if (dirFisica != UINT32_MAX) {
        log_info(logger,"Dirección Física: %u\n", dirFisica);
    } else {
        log_info(logger,"TLB Miss\n");
    }*/
    
   // return 0;
	

	free (nombre);
	/*free(page_table->entries);
    free(page_table);*/ //TODO: Liberar
	
	return EXIT_SUCCESS;
}