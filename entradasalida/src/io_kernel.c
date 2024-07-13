#include "../include/io_kernel.h"
#define BUFFER_SIZE 1024

void io_atender_kernel(){
	bool control=1;
    //int size;
	while (control) {
    	int cod_op = recibirOperacion(fd_kernel);
    	switch (cod_op) {
    	case MENSAJE:
   	 		recibirMensaje(fd_kernel);
			// char mensajeConexion[BUFFER_SIZE] = {0};  

			
			// int bytes_recibidos = recv(fd_kernel, mensajeConexion, sizeof(mensajeConexion), 0);
			
			// if (bytes_recibidos < 0) {
			// 	perror("Error al recibir el mensaje");
			// 	return NULL;
			// }
			// mensajeConexion[bytes_recibidos] = '\0'; // Asegurar el carácter nulo al final del mensaje
			
			// log_info(logger, "valor recibido: %s", mensajeConexion);
   	 		break;
    		/*//case PAQUETE:
   		 lista = recibir_paquete(fd_io);
   		 //log_info(memoria_logger, "Me llegaron los siguientes valores:\n");
   	 	//list_iterate(lista, (void*) iterator);
   	  //   break;*/
	  	case READ:
		log_info(logger, "stdin read recibido");
		manejarSTDINRead(fd_kernel);
		break;
   		case IO_GEN_SLEEP:
   			
			recibir_mensaje_y_dormir(fd_kernel);
   			break;
		case IO_STDIN_READ:
   			
			recibir_mensaje_y_dormir(fd_kernel);
   			break;
		case IO_STDOUT_WRITE:
   			
			recibir_mensaje_y_dormir(fd_kernel);
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

void manejarSTDINRead(int socketCliente) {
      t_list* lista = recibirPaquete(socketCliente);
    if (lista == NULL) {
        log_error(logger, "Error al recibir paquete");
        return;
    }

    // Procesar la lista de valores según tu lógica
    char* direccion = list_get(lista, 0); // Por ejemplo, obtener el primer parámetro
    char* tamanioTexto = list_get(lista, 1); // Por ejemplo, obtener el segundo parámetro

    // Loguear los parámetros recibidos
    log_info(logger, "Tamanio recibido: %s", tamanioTexto);
    log_info(logger, "Direccion recibida: %s", direccion);

    // Liberar memoria del paquete
    list_destroy_and_destroy_elements(lista, free);
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
	  log_info(logger, "Tiempo a dormir recibido: %d", unidades ); 
	  log_info(logger, "Tiempo a dormir calculado: %f", unidades*TIEMPO_UNIDAD_TRABAJO/1000.0); // ejemplo: 10*250/1000 = 2.5seg
	  log_info(logger, "antes de dormir");
	  sleep(unidades* TIEMPO_UNIDAD_TRABAJO/1000.0);
  
	  //mandar mensaje luego de dormir a kernel
	  char* respuesta = "OK";
    send(socket_cliente, respuesta, strlen(respuesta), 0);

}