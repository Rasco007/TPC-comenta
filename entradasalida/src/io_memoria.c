#include "../include/io_memoria.h"


void io_atender_memoria(){
	bool control=1;
    //int size;
//while (control) {
    	int cod_op = recibirOperacion(fd_memoria);
    	switch (cod_op) {
    	case MENSAJE: //STDOUT
   	 		char *recibido=recibirMensaje(fd_memoria);
			log_info(logger, "valor recibido: %s", recibido);
   	 		break;

    	case -1:
   			log_error(logger, "Memoria se desconectó. Terminando servidor");
   			control = 0;
   			break;
    	default:
   		 	log_warning(logger,"Operacion desconocida. No quieras meter la pata");
   	 		break;
    	}
//	}
}