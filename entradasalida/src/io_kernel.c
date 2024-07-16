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
   			
			manejarSTDOUTRead(fd_kernel);
   			break;
		case IO_FS_CREATE:
			//TODO
			break;
		case IO_FS_DELETE:
			//TODO
			break;
		case IO_FS_READ:
			//TODO
			break;
		case IO_FS_TRUNCATE:
			//TODO
			break;
		case IO_FS_WRITE:
			//TODO
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
void enviarAImprimirAMemoria(const char *mensaje, int direccion, int socket) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 100;
    paquete->buffer = malloc(sizeof(t_buffer));
    size_t mensaje_len = strlen(mensaje) ; // +1 para el terminador nulo??????
    paquete->buffer->size = sizeof(int) + mensaje_len;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), mensaje, mensaje_len);
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); // Manejo de error, puedes ajustarlo según tu aplicación
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(a_enviar);
    free(paquete);
}

void enviarDireccionTamano(int direccion,int tamano, int socket) {
   t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 101;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 2 * sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &direccion, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &tamano, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    void *a_enviar = serializarPaquete(paquete, bytes);
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); // Manejo de error, puedes ajustarlo según tu aplicación
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(a_enviar);
    free(paquete);
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
    // Leer una línea de texto usando readline
    char* texto = readline("Ingrese el texto: ");
	char *datosLeidos = (char *)malloc(atoi(tamanioTexto) + 1); // +1 para el terminador nulo
    if (datosLeidos == NULL) {
        perror("Error al reservar memoria para los datos leídos");
        return ;
    }
    // Copiar los datos desde el archivo mapeado al buffer de datos leídos
    memcpy(datosLeidos, texto, atoi(tamanioTexto));
	printf("Texto a enviar: %s\n", datosLeidos);
	enviarAImprimirAMemoria(datosLeidos,atoi(direccion), fd_memoria); //estos datos se deben escribir en la direccion de memoria
	//UNA FUNCION QUE MANDE "datosLeidos" A MEMORIA Y LO ESCRIBA EN "direccion"
	// Liberar la memoria reservada
	free(texto);
	free(datosLeidos);
    // Liberar memoria del paquete
    list_destroy_and_destroy_elements(lista, free);
}

void manejarSTDOUTRead(int socketCliente) {
    t_list* lista = recibirPaquete(socketCliente);
    if (lista == NULL) {
        log_error(logger, "Error al recibir paquete");
        return;
    }
    // Procesar la lista de valores según tu lógica
    char* nombre = list_get(lista, 0);
	char* direccion = list_get(lista, 1);
    char* tamanioTexto = list_get(lista, 2);
    // Loguear los parámetros recibidos
    log_info(logger, "Tamanio recibido: %s", tamanioTexto);
    log_info(logger, "Direccion recibida: %s", direccion);
	log_info(logger, "nombre: %s", nombre);
	direccion="1"; //VER CUANDO SE TRADUCE DE DL A DF
	tamanioTexto="2";
	enviarDireccionTamano(atoi(direccion), atoi(tamanioTexto),fd_memoria);
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
	log_info(logger, "Despues de dormir");
	//mandar mensaje luego de dormir a kernel
	// char* respuesta = "OK";
    send(socket_cliente, "OK", 2, 0);
	//enviarMensaje("OK", socket_cliente);
	return;
}