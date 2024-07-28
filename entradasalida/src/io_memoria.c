#include "../include/io_memoria.h"
extern int tamaniosglobales[20];
extern int cantidadglobal;
void io_atender_memoria(){
	/*bool control=1;
	int i=0;
	char* cadenaCompleta=malloc(126);
	int longitud=0;
    //int size;
	while (control) {
    	int cod_op = recibirOperacion(fd_memoria);
    	switch (cod_op) {
    	case MENSAJE: //STDOUT
			if(strcmp(TIPO_INTERFAZ, "STDOUT") == 0 || strcmp(TIPO_INTERFAZ, "DialFS") == 0)
   	 		{
				char* recibido="";
				if(i==0)
					cadenaCompleta[0]='\0';
				recibido= recibirMensaje(fd_memoria);
				printf("Mensaje recibido de memoria:%s\n", recibido);
				//ir concatenando los mensajes
				strncat(cadenaCompleta, recibido, tamaniosglobales[i]);
				longitud+=tamaniosglobales[i];
				i++;
				if(i==cantidadglobal){
					printf("Mensaje completo:%s\n", cadenaCompleta);
					if (strcmp(TIPO_INTERFAZ, "DialFS") == 0) 
						escribirCadenaEnArchivo(archivoWrite, cadenaCompleta, pointerArchivo);
					free(cadenaCompleta);
					char *mensje="ok";
    				send(fd_kernel, &mensje, sizeof(mensje), 0);
					i=0;
			}
			//free(recibido);
			}
			//else
			//	enviarMensaje("OK", fd_kernel); 
   	 		break;

    	case -1:
   			log_error(logger, "Memoria se desconectó. Terminando servidor");
   			control = 0;
   			break;
    	default:
   		 	log_warning(logger,"Operacion desconocida. No quieras meter la pata");
   	 		break;
    	}
	}*/
}