/*- Interpreta y ejecuta las instrucciones de los Contextos de Ejecución recibidos por parte del Kernel. 
Para ello, ejecutará un ciclo de instrucción con los pasos: Fetch, Decode, Execute y Check Interrupt.
- A la hora de ejecutar instrucciones que lo requieran, traducira las direcciones lógicas (propias del proceso) 
a direcciones físicas (propias de la memoria) simulando la existencia de una MMU.
- Durante el transcurso de la ejecución de un proceso, se irá actualizando su Contexto de Ejecución, 
que luego será devuelto al Kernel bajo los siguientes escenarios: finalización del mismo (instrucción EXIT o ante un error), 
solicitar una llamada al Kernel, o deber ser desalojado (interrupción).*/

#include <main/cpu.h>

uint64_t tiempo_actual = 0;
TLB tlb;
PageTable *page_table;

void inicializar_tlb() {
    tlb.size = 32; //todo: tiene que agarrar el CANTIDAD_ENTRADAS_TLB
    for (size_t i = 0; i < 32; i++) {
        tlb.entries[i].valid = false;
    }
	log_info(logger, "TLB inicializada. Primer entrada: %d", tlb.entries[0].valid);
}

int main(void){
	
	logger = iniciarLogger("cpu.log", "CPU");
	loggerError = iniciarLogger("errores.log", "Errores CPU");

	config = iniciarConfiguracion("cpu.config");

	atexit(terminarPrograma);

	conexionMemoria(); 

	char * nombre = string_duplicate("CPU-KERNEL");
	cambiarNombre(logger, nombre);

	inicializar_tlb(); 

	mmu("12345", 32, tlb);

    escucharAlKernel();

	
	

	free (nombre);
	/*free(page_table->entries);
    free(page_table);*/ //TODO: Liberar
	
	return EXIT_SUCCESS;
}