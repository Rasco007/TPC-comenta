#include "../include/io_kernel.h"
#define BUFFER_SIZE 1024
char* archivoWrite;
int pointerArchivo;
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
   			log_info(logger, "sleep recibido");
			recibir_mensaje_y_dormir(fd_kernel);
   			break;
		case IO_STDIN_READ:
   			log_info(logger, "stdin write recibido");
			manejarSTDINRead(fd_kernel);
   			break;
		case IO_STDOUT_WRITE:
   			log_info(logger, "stdout write recibido");
			manejarSTDOUTRead(fd_kernel);
   			break;
		case IO_FS_CREATE:
            log_info(logger, "fs create recibido");
			manejarFS_CREATE(fd_kernel);
			break;
		case IO_FS_DELETE:
			log_info(logger, "fs delete recibido");
            manejarFS_DELETE(fd_kernel);   
			break;
		case IO_FS_READ:
			log_info(logger, "fs read recibido");
            manejarFS_READ(fd_kernel);
			break;
		case IO_FS_TRUNCATE:
			log_info(logger, "fs truncate recibido");
            manejarFS_TRUNCATE(fd_kernel);
			break;
		case IO_FS_WRITE:
			log_info(logger, "fs write recibido");
            manejarFS_WRITE(fd_kernel);
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

void manejarFS_DELETE(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2;    
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&longitud2, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    printf("Longitud de la cadena recibida: %d\n", longitud2);
    // Copiar la cadena recibida
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 2*sizeof(int)+longitud1, longitud2);
    // Asegurarse de que la cadena esté terminada en nulo
    nombrearchivo[longitud2] = '\0';
    printf("Nombre de archivo: %s\n", nombrearchivo);
    delete_file(nombrearchivo);
}

void manejarFS_TRUNCATE(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2;    
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    int  nuevoTamanio;
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&nuevoTamanio, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    printf("Nuevo tamanio: %d\n", nuevoTamanio);
    memcpy(&longitud2, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int));
    printf("Longitud de la cadena recibida: %d\n", longitud2);
    // Copiar la cadena recibida
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 3*sizeof(int)+longitud1, longitud2);
    // Asegurarse de que la cadena esté terminada en nulo
    nombrearchivo[longitud2] = '\0';
    printf("Nombre de archivo: %s\n", nombrearchivo);
    truncarArchivo2(nombrearchivo,nuevoTamanio);
}
void manejarFS_READ(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2, direccion, tamanio, punteroArchivo;    
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&direccion, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    printf("direccion: %d\n", direccion);
    memcpy(&tamanio, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int));
    printf("tamanio: %d\n", tamanio);
    memcpy(&punteroArchivo, buffer + sizeof(op_code)+3*sizeof(int)+longitud1, sizeof(int));
    printf("pointer: %d\n", punteroArchivo);
    memcpy(&longitud2, buffer + sizeof(op_code) + 4*sizeof(int)+longitud1, sizeof(int));
    printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 5*sizeof(int)+longitud1, longitud2);
    nombrearchivo[longitud2] = '\0';
    printf("Nombre de archivo: %s\n", nombrearchivo);
    archivoWrite=nombrearchivo;
    pointerArchivo=punteroArchivo;
    char *datosLeidos= leerDatosDesdeArchivo(nombrearchivo, punteroArchivo, tamanio);
    printf("Datos leidos: %s\n", datosLeidos);
    direccion=16;
    enviarAImprimirAMemoria(datosLeidos,direccion,fd_memoria);
}

void manejarFS_WRITE(int socketCliente){
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2, direccion, tamanio, punteroArchivo;    
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&direccion, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    printf("direccion: %d\n", direccion);
    memcpy(&tamanio, buffer + sizeof(op_code)+2*sizeof(int)+longitud1, sizeof(int));
    printf("tamanio: %d\n", tamanio);
    memcpy(&punteroArchivo, buffer + sizeof(op_code)+3*sizeof(int)+longitud1, sizeof(int));
    printf("pointer: %d\n", punteroArchivo);
    memcpy(&longitud2, buffer + sizeof(op_code) + 4*sizeof(int)+longitud1, sizeof(int));
    printf("Longitud de la cadena recibida: %d\n", longitud2);
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 5*sizeof(int)+longitud1, longitud2);
    nombrearchivo[longitud2] = '\0';
    printf("Nombre de archivo: %s\n", nombrearchivo);
    archivoWrite=nombrearchivo;
    pointerArchivo=punteroArchivo;
    enviarDireccionTamano(direccion,tamanio,fd_memoria);
}

void manejarFS_CREATE(int socketCliente) {
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    if (bytes_recibidos == 0) {
        printf("Conexión cerrada por el servidor\n");
        return;
    }
    // Asegurarse de que tenemos suficientes datos para los campos esperados
    if (bytes_recibidos < sizeof(int) + sizeof(op_code)) {
        fprintf(stderr, "Mensaje recibido incompleto\n");
        return;
    }
    int longitud1,longitud2;    
    char nombreinterfaz[2048];
    char nombrearchivo[2048];
    memcpy(&longitud1, buffer + sizeof(op_code), sizeof(int)); 
    printf("Longitud de la cadena recibida: %d\n", longitud1);
    memcpy(&nombreinterfaz, buffer + sizeof(op_code) + sizeof(int), longitud1);
    nombreinterfaz[longitud1] = '\0';   
    printf("Nombre de interfaz recibido: %s\n", nombreinterfaz);
    memcpy(&longitud2, buffer + sizeof(op_code)+sizeof(int)+longitud1, sizeof(int));
    printf("Longitud de la cadena recibida: %d\n", longitud2);
    // Copiar la cadena recibida
    memcpy(&nombrearchivo, buffer + sizeof(op_code) + 2*sizeof(int)+longitud1, longitud2);
    // Asegurarse de que la cadena esté terminada en nulo
    nombrearchivo[longitud2] = '\0';
    printf("Nombre de archivo: %s\n", nombrearchivo);
    crearArchivo2(nombrearchivo);
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
        exit(EXIT_FAILURE); // Manejo de error
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
void recibirEnteros3(int socket, int *pid, int *indice) {
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    memcpy(pid ,buffer+sizeof(op_code), sizeof(int));
    memcpy(indice, buffer+sizeof(int)+sizeof(op_code), sizeof(int));
}

void manejarSTDINRead(int socketCliente) {
    int tamanioTexto;
    int direccion;
    recibirEnteros3(socketCliente, &tamanioTexto, &direccion);
    // Loguear los parámetros recibidos
    log_info(logger, "Tamanio recibido: %d", tamanioTexto);
    log_info(logger, "Direccion recibida: %d", direccion);
    // Leer una línea de texto usando readline
    char* texto = readline("Ingrese el texto: ");
	char *datosLeidos = (char *)malloc(tamanioTexto ); // +1 para el terminador nulo
    datosLeidos[tamanioTexto] = '\0'; // Asegurar el terminador nulo
    if (datosLeidos == NULL) {
        perror("Error al reservar memoria para los datos leídos");
        return ;
    }
    // Copiar los datos desde el archivo mapeado al buffer de datos leídos
    memcpy(datosLeidos, texto, tamanioTexto);
	printf("Texto a enviar: %s\n", datosLeidos);
	enviarAImprimirAMemoria(datosLeidos,direccion, fd_memoria); //estos datos se deben escribir en la direccion de memoria
	//UNA FUNCION QUE MANDE "datosLeidos" A MEMORIA Y LO ESCRIBA EN "direccion"
	// Liberar la memoria reservada
	free(texto);
	free(datosLeidos);
}

void manejarSTDOUTRead(int socketCliente) {
    int tamanioTexto;
    int direccion;
    recibirEnteros3(socketCliente, &tamanioTexto, &direccion);
    // Loguear los parámetros recibidos
    log_info(logger, "Tamanio recibido: %d", tamanioTexto);
    log_info(logger, "Direccion recibida: %d", direccion);
	//log_info(logger, "nombre: %s", nombre);
	//direccion="1"; //VER CUANDO SE TRADUCE DE DL A DF
	//tamanioTexto="2";
	enviarDireccionTamano(direccion, tamanioTexto,fd_memoria);
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