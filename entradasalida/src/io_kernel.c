#include "../include/io_kernel.h"

void io_atender_kernel(){
	bool control=1;
    //int size;
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
   			/*recibirBuffer(fd_kernel,&size);
        	log_info(logger, "antes de dormir");
   			sleep(5 * TIEMPO_UNIDAD_TRABAJO/1000);
        	log_info(logger, "dps de dormir");
   			enviarMensaje("Terminó I/O", fd_kernel);*/
			recibir_mensaje_y_dormir(fd_kernel);
   			break;
		case IO_STDIN_READ:
			hacer_funcion_stdin(fd_kernel);
   			break;
		case IO_STDOUT_WRITE:
			hacer_funcion_stdout(fd_kernel);
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

void hacer_funcion_stdin(int socket_cliente) {

	leer_por_consola();
	printf('hacer funcion stdin');
    
}

void hacer_funcion_stdout(int socket_cliente) {
	printf('hacer funcion stdout');
    
}


void recibir_mensaje_y_dormir(int socket_cliente) {
    // Buffer para almacenar el mensaje recibido
    char buffer[1024];
    
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket_cliente, buffer, sizeof(buffer), 0);
    
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }

    // Interpretar el mensaje recibido como un entero
    int unidades;
    memcpy(&unidades, buffer, sizeof(int));
	// Extraer el nombre del inicio del buffer
    char* nombre = buffer + sizeof(int);
    // Asegurarse de que el nombre esté correctamente terminado por un carácter nulo
    nombre[bytes_recibidos - sizeof(int)] = '\0';
	
	log_info(logger, "Nombre recibido: %s", nombre);
	log_info(logger, "Tiempo a dormir recibido: %f", unidades*TIEMPO_UNIDAD_TRABAJO/1000.0); // ejemplo: 10*250/1000 = 2.5seg
	log_info(logger, "antes de dormir");
	sleep(unidades* TIEMPO_UNIDAD_TRABAJO/1000.0);
	enviarMensaje("Terminó I/O", fd_kernel);
	log_info(logger, "dps de dormir");
	
    
}

void leer_por_consola() {
	
  log_info(logger,"Por favor ingrese un texto y presione Enter.");
    // Leer la línea de entrada del usuario
    char *input = readline(">");

    // Verificar si se ha recibido algo
    if (input != NULL) {
        // Imprimir lo ingresado
        log_info(logger, "Has ingresado: %s\n", input);
        
        // Liberar la memoria asignada para la entrada
        free(input);
    } else {
        log_error(loggerError,"Error al leer la entrada.\n");
    }
    
}