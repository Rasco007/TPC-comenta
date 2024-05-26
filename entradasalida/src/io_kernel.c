#include "../include/io_kernel.h"


void io_atender_kernel(){
	bool control=1;
    int size;
	while (control) {
    	int cod_op = recibirOperacion(fd_kernel);
    	switch (cod_op) {
    	case MENSAJE:
   	 	recibirMensaje(fd_kernel);
   	 	break;
    	/*//case PAQUETE:
   		 lista = recibir_paquete(fd_io);
   		 //log_info(memoria_logger, "Me llegaron los siguientes valores:\n");
   	 	//list_iterate(lista, (void*) iterator);
   	  //   break;*/
   	 case IO_GEN_SLEEP:
   		 recibirBuffer(fd_kernel,&size);
        log_info(logger, "antes de dormir");
   		 sleep(5 * TIEMPO_UNIDAD_TRABAJO/1000);
        log_info(logger, "dps de dormir");
   		 enviarMensaje("Terminó I/O", fd_kernel);
   		 break;
    	case -1:
   		 log_error(logger, "Kernel se desconectó. Terminando servidor");
   		 control = 0;
   		 break;
    	default:
   	 	log_warning(logger,"Operacion desconocida. No quieras meter la pata");
   	 	break;
    	}
	}
}
