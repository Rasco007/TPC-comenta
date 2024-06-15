#include <peticiones/manejoPaginas.h>

void recibirTablaDePaginasActualizada(t_pcb* pcb){

	int size, desplazamiento = 0;
	void * buffer;

	buffer = recibirBuffer(conexionAMemoria, &size);

    desplazamiento += sizeof(int);
    
    desplazamiento += sizeof(uint32_t) + sizeof(int);
    
    t_pagina* pagina;

    list_clean_and_destroy_elements (pcb->tablaDePaginas, free);
    
    uint32_t tablaDePaginasSize;
    
    memcpy(&(tablaDePaginasSize), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    
    for (uint32_t i = 0; i < tablaDePaginasSize; i++) {

        pagina = deserializarPagina(buffer, &desplazamiento);
        list_add (pcb->tablaDePaginas, pagina);
    }
    free (buffer);
    desplazamiento += sizeof(int);

}

uint32_t recibirPID(int socketCliente) {

	int size, desplazamiento=0; 
	uint32_t pid; 

	void* buffer = recibirBuffer(socketCliente, &size);
	memcpy(&(pid), buffer + desplazamiento, sizeof(uint32_t));

	free (buffer);
	return pid; 
}

