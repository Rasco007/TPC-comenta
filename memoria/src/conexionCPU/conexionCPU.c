#include <conexionCPU/conexionCPU.h>

int tiempo;
char* valorLeido; 

int ejecutarServidorCPU(int * socketCliente){

	tiempo = config_get_int_value(config,"RETARDO_MEMORIA");

	while (1) {
		int peticion = recibirOperacion(*socketCliente);
		debug("Se recibio peticion %d del CPU", peticion); 
	}
} 





