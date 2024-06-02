#include <peticiones/peticiones.h>

//serializar tabla de Segmentos

void enviarTablaPaginas(t_proceso* procesoEnMemoria){ 
   return 0;
}


uint32_t recibirPID(int socketCliente) {

	int size, desplazamiento=0; 
	uint32_t pid; 

	void* buffer = recibirBuffer(socketCliente, &size);
	desplazamiento += sizeof(int);
	memcpy(&(pid), buffer + desplazamiento, sizeof(uint32_t));

	free (buffer);
	return pid; 

}

