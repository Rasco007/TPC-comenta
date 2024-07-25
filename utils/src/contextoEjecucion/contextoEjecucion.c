#include <contextoEjecucion/contextoEjecucion.h>
t_contexto * contextoEjecucion = NULL;
t_dictionary *crearDiccionarioDeRegistros2();

// MANEJO DE CONTEXTO
void enviarContextoBeta(int socket, t_contexto* contexto) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = CONTEXTOEJECUCION;
    paquete->buffer = malloc(sizeof(t_buffer));

    // Calcular el tamaño del buffer necesario para la estructura
    paquete->buffer->size = sizeof(contexto->pid) + sizeof(contexto->programCounter) + sizeof(contexto->instruccionesLength) ;

     // Calcular el tamaño de los registros
    paquete->buffer->size  += 4 * (3 + 10); // AX, BX, CX, DX (1 bytes cada uno) + EAX, EBX, ECX, EDX (4 bytes cada uno)
    
    //Calculo para SI y DI
    paquete->buffer->size+=sizeof(contexto->SI)+sizeof(contexto->DI);

    //calcula tamaño del motivo
    paquete->buffer->size  += sizeof(contextoEjecucion->motivoDesalojo->motivo) +
                   sizeof(contextoEjecucion->motivoDesalojo->parametrosLength);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        paquete->buffer->size  += strlen(contextoEjecucion->motivoDesalojo->parametros[i]) + 1; // +1 para el terminador nulo
    }

    //calculo tamaño de tiempo de cpu
    paquete->buffer->size  += sizeof(contextoEjecucion->tiempoDeUsoCPU);

    //calculo para quantum y algoritmo
    paquete->buffer->size+=sizeof(contexto->quantum)+sizeof(contexto->algoritmo);

    paquete->buffer->stream = malloc(paquete->buffer->size);

    // Serializar los datos en el buffer
    int desplazamiento = 0;
    //agregar a paquete
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->pid), sizeof(contexto->pid));
    desplazamiento += sizeof(contexto->pid);

    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->programCounter), sizeof(contexto->programCounter));
    desplazamiento += sizeof(contexto->programCounter);
 
    log_info(logger, "cantidad de instrucciones mandadas %d",contexto->instruccionesLength);
   memcpy(paquete->buffer->stream + desplazamiento, &(contexto->instruccionesLength), sizeof(contexto->instruccionesLength));
    desplazamiento += sizeof(contexto->instruccionesLength);

    // Serializar los registros
    char* registros[] = {"AX", "BX", "CX", "DX", "EAX", "EBX", "ECX", "EDX"};
    for (int i = 0; i < 8; i++) {
        char* registro = dictionary_get(contexto->registrosCPU, registros[i]);
        int registro_length = (i < 4) ? 3 : 10;
        memcpy(paquete->buffer->stream + desplazamiento, registro, registro_length);
        desplazamiento += registro_length;
    }

    // Serializar SI y DI
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->SI), sizeof(contexto->SI));
    desplazamiento += sizeof(contexto->SI);
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->DI), sizeof(contexto->DI));
    desplazamiento += sizeof(contexto->DI);

     // Serializar el motivo
    memcpy(paquete->buffer->stream + desplazamiento, &(contextoEjecucion->motivoDesalojo->motivo), sizeof(contextoEjecucion->motivoDesalojo->motivo));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->motivo);

    
    memcpy(paquete->buffer->stream + desplazamiento, &(contextoEjecucion->motivoDesalojo->parametrosLength), sizeof(contextoEjecucion->motivoDesalojo->parametrosLength));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->parametrosLength);

    
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        int parametro_length = strlen(contextoEjecucion->motivoDesalojo->parametros[i]) + 1; // +1 para el terminador nulo
        memcpy(paquete->buffer->stream + desplazamiento, contextoEjecucion->motivoDesalojo->parametros[i], parametro_length);
        desplazamiento += parametro_length;
    }

    //Serializacion de quantum
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->quantum), sizeof(contexto->quantum));
    desplazamiento += sizeof(contexto->quantum);

    //Serializacion de algoritmo
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->algoritmo),sizeof(contexto->algoritmo));
    desplazamiento +=sizeof(contexto->algoritmo);
    log_info(logger,"ALGORITMO: %d", contexto->algoritmo);

    //serializo el tiempo de cpu
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->tiempoDeUsoCPU), sizeof(contexto->tiempoDeUsoCPU));
    desplazamiento += sizeof(contexto->tiempoDeUsoCPU);

log_info(logger,"---------------------");
    // Calcular el tamaño total del paquete a enviar
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
    
    // Serializar el paquete
    void *a_enviar = serializarPaquete(paquete, bytes);

    // Enviar el paquete a través del socket
    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE);
    }

    // Liberar memoria
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(a_enviar);
    free(paquete);
}




void recibirContextoBeta(int socket) {
       char buffer[2048];
        if (contextoEjecucion != NULL) destroyContextoUnico ();
	iniciarContexto ();

    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }

    

    int desplazamiento = sizeof(op_code);

    memcpy(&(contextoEjecucion->pid), buffer + desplazamiento, sizeof(contextoEjecucion->pid));
    desplazamiento += sizeof(contextoEjecucion->pid);

    memcpy(&(contextoEjecucion->programCounter), buffer + desplazamiento, sizeof(contextoEjecucion->programCounter));
    desplazamiento += sizeof(contextoEjecucion->programCounter);

    

    memcpy(&(contextoEjecucion->instruccionesLength), buffer + desplazamiento, sizeof(contextoEjecucion->instruccionesLength));
    desplazamiento += sizeof(contextoEjecucion->instruccionesLength);
    log_info(logger,"cantidad de instrucciones RECIBIDAS %u", contextoEjecucion->instruccionesLength);

    printf("Recibido PID: %u PC: %d \n", contextoEjecucion->pid, contextoEjecucion->programCounter);
    

        //deserealizo los registros
     char* registro;
    char nombreRegistro[4];

    // AX, BX, CX, DX (1 byte cada uno)
    for (char nombre = 'A'; nombre <= 'D'; nombre++) {
        registro = malloc(4); // 1 byte + terminador nulo
        memcpy(registro, buffer + desplazamiento, 3);
        registro[3] = '\0';
        desplazamiento += 3;

        snprintf(nombreRegistro, 3, "%cX", nombre);
        dictionary_put(contextoEjecucion->registrosCPU, nombreRegistro, registro);
        log_info(logger, "Registro %s: %s", nombreRegistro, registro);
        //free(registro);
    }

    // EAX, EBX, ECX, EDX (4 bytes cada uno)
    for (char nombre = 'A'; nombre <= 'D'; nombre++) {
        registro = malloc(11); // 4 bytes + terminador nulo
        memcpy(registro, buffer + desplazamiento, 10);
        registro[10] = '\0';
        desplazamiento += 10;

        snprintf(nombreRegistro, 4, "E%cX", nombre);
        dictionary_put(contextoEjecucion->registrosCPU, nombreRegistro, registro);
        log_info(logger, "Registro %s: %s", nombreRegistro, registro);
        //free(registro);
    }

    // Deserializar SI y DI
    memcpy(&(contextoEjecucion->SI), buffer + desplazamiento, sizeof(contextoEjecucion->SI));
    desplazamiento += sizeof(contextoEjecucion->SI);
    memcpy(&(contextoEjecucion->DI), buffer + desplazamiento, sizeof(contextoEjecucion->DI));
    desplazamiento += sizeof(contextoEjecucion->DI);

      // Deserializar el motivo
    memcpy(&(contextoEjecucion->motivoDesalojo->motivo), buffer + desplazamiento, sizeof(contextoEjecucion->motivoDesalojo->motivo));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->motivo);

    // Deserializar la longitud de los parámetros
    memcpy(&(contextoEjecucion->motivoDesalojo->parametrosLength), buffer + desplazamiento, sizeof(contextoEjecucion->motivoDesalojo->parametrosLength));
    desplazamiento += sizeof(contextoEjecucion->motivoDesalojo->parametrosLength);

    // Deserializar los parámetros
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        int parametro_length = strlen(buffer + desplazamiento) + 1; // +1 para el terminador nulo
        contextoEjecucion->motivoDesalojo->parametros[i] = malloc(parametro_length);
        memcpy(contextoEjecucion->motivoDesalojo->parametros[i], buffer + desplazamiento, parametro_length);
        desplazamiento += parametro_length;
        log_info(logger, "param %s",contextoEjecucion->motivoDesalojo->parametros[i]);
    }
    
    //Deserializar quantum
    memcpy(&(contextoEjecucion->quantum), buffer + desplazamiento, sizeof(contextoEjecucion->quantum));
    desplazamiento += sizeof(contextoEjecucion->quantum);

    //Deserializar algoritmo
    memcpy(&(contextoEjecucion->algoritmo), buffer + desplazamiento, sizeof(contextoEjecucion->algoritmo));
    desplazamiento += sizeof(contextoEjecucion->algoritmo);
    log_info(logger,"ALGORITMO: %d", contextoEjecucion->algoritmo);

    //Deserializar tiempoDeUsoCPU
    memcpy(&(contextoEjecucion->tiempoDeUsoCPU), buffer + desplazamiento, sizeof(contextoEjecucion->tiempoDeUsoCPU));
    desplazamiento += sizeof(contextoEjecucion->tiempoDeUsoCPU);

    log_info(logger, "termino de recibir todo");
    
}


void iniciarContextoBeta(){

    contextoEjecucion = malloc(sizeof(t_contexto));
	contextoEjecucion->instrucciones = list_create();
	contextoEjecucion->instruccionesLength = 0;
	contextoEjecucion->pid = 0;
	contextoEjecucion->programCounter = 0;
	contextoEjecucion->registrosCPU = dictionary_create();
	contextoEjecucion->tablaDePaginas = list_create();
	contextoEjecucion->tablaDePaginasSize = 0;
    contextoEjecucion->tiempoDeUsoCPU = 0;
    contextoEjecucion->motivoDesalojo = (t_motivoDeDesalojo *)malloc(sizeof(t_motivoDeDesalojo));
    contextoEjecucion->motivoDesalojo->parametros[0] = "param1";
    contextoEjecucion->motivoDesalojo->parametros[1] = "param2";
    contextoEjecucion->motivoDesalojo->parametros[2] = "param3";
    contextoEjecucion->motivoDesalojo->parametrosLength = 3;
    contextoEjecucion->motivoDesalojo->motivo = IO_GEN_SLEEP;
	
}

void destroyContexto() {
    //list_destroy_and_destroy_elements(contextoEjecucion->instrucciones, free);
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    //list_destroy_and_destroy_elements(contextoEjecucion->tablaDePaginas, free);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) 
        if (strcmp(contextoEjecucion->motivoDesalojo->parametros[i], "")) free(contextoEjecucion->motivoDesalojo->parametros[i]);
    free(contextoEjecucion->motivoDesalojo);
    free(contextoEjecucion);
    contextoEjecucion = NULL;
}

void destroyContextoUnico () {
    //list_destroy(contextoEjecucion->instrucciones);
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    //list_destroy(contextoEjecucion->tablaDePaginas);
    /*for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) 
        if (strcmp(contextoEjecucion->motivoDesalojo->parametros[i], "")) 
            free(contextoEjecucion->motivoDesalojo->parametros[i]);*/
    free(contextoEjecucion->motivoDesalojo);
    free(contextoEjecucion);
    
    contextoEjecucion = NULL;
}

t_dictionary *crearDiccionarioDeRegistros2(){

    t_dictionary *registros = dictionary_create();

    char name[3] = "AX", longName[4] = "EAX";
    for (int i = 0; i < 4; i++) {
        dictionary_put(registros, name, string_repeat('0', 4));
        longName[0] = 'E';
        dictionary_put(registros, longName, string_repeat('0', 8));
        name[0]++, longName[1]++;
    }

    return registros;
}

void iniciarContexto(){

    contextoEjecucion = malloc(sizeof(t_contexto));
	//contextoEjecucion->instrucciones = list_create();
	contextoEjecucion->instruccionesLength = 0;
	contextoEjecucion->pid = 0;
    contextoEjecucion->SI=0;
    contextoEjecucion->DI=0;
	contextoEjecucion->programCounter = 0;
	contextoEjecucion->registrosCPU = dictionary_create();
	//contextoEjecucion->tablaDePaginas = list_create();
	//contextoEjecucion->tablaDePaginasSize = 0;
    contextoEjecucion->tiempoDeUsoCPU = 0;
    contextoEjecucion->motivoDesalojo = (t_motivoDeDesalojo *)malloc(sizeof(t_motivoDeDesalojo));
    contextoEjecucion->motivoDesalojo->parametros[0] = "";
    contextoEjecucion->motivoDesalojo->parametros[1] = "";
    contextoEjecucion->motivoDesalojo->parametros[2] = "";
    contextoEjecucion->motivoDesalojo->parametros[3] = "";
    contextoEjecucion->motivoDesalojo->parametros[4] = "";
    contextoEjecucion->motivoDesalojo->parametrosLength = 0;
    contextoEjecucion->motivoDesalojo->motivo = 0;
	contextoEjecucion->quantum=0;
    contextoEjecucion->algoritmo=0;
}



void enviarContextoActualizado(int socket){ 
    t_paquete * paquete = crearPaquete();
    
    paquete->codigo_operacion = CONTEXTOEJECUCION;
    
   
    agregarAPaquete (paquete,(void *)&contextoEjecucion->pid, sizeof(contextoEjecucion->pid));
    agregarAPaquete (paquete,(void *)&contextoEjecucion->programCounter, sizeof(contextoEjecucion->programCounter));
    
    agregarInstruccionesAPaquete (paquete, contextoEjecucion->instrucciones);
    //log_info(logger, "Instrucciones agregadas al paquete");

    agregarRegistrosAPaquete(paquete, contextoEjecucion->registrosCPU);
    //log_info(logger, "Registros agregados al paquete");
    
    agregarTablaDePaginasAPaquete(paquete);
    //log_info(logger, "Tabla de paginas agregada al paquete");

    agregarMotivoAPaquete(paquete, contextoEjecucion->motivoDesalojo);
    //log_info(logger, "Motivo de desalojo agregado al paquete");

    agregarAPaquete(paquete, (void *)&contextoEjecucion->tiempoDeUsoCPU, sizeof(contextoEjecucion->tiempoDeUsoCPU));
    //log_info(logger, "Rafaga de CPU agregada al paquete");

    enviarPaquete(paquete, socket);
    //log_info(logger, "Procede a eliminar paquete");
	eliminarPaquete(paquete);
}

//Ver
void agregarTablaDePaginasAPaquete(t_paquete* paquete){
    
    agregarAPaquete (paquete, &(contextoEjecucion->tablaDePaginasSize), sizeof contextoEjecucion->tablaDePaginasSize);

    uint32_t i;
    for(i=0;i<contextoEjecucion->tablaDePaginasSize;i++){
        agregarPaginaAPaquete(paquete,list_get(contextoEjecucion->tablaDePaginas, i));
    }
}

//Ver
void agregarPaginaAPaquete(t_paquete* paquete, t_pagina* pagina){
	agregarAPaquete(paquete, &(pagina->idPagina), sizeof(uint32_t));
	agregarAPaquete(paquete, &(pagina->idFrame), sizeof(uint32_t));
	agregarAPaquete(paquete, &(pagina->bitDeValidez), sizeof(uint32_t));
}

void agregarMotivoAPaquete (t_paquete* paquete, t_motivoDeDesalojo* motivoDesalojo){

    agregarAPaquete (paquete, (void *)&motivoDesalojo->motivo, sizeof (motivoDesalojo->motivo));
    agregarAPaquete (paquete, (void *)&motivoDesalojo->parametrosLength, sizeof (motivoDesalojo->parametrosLength));
    for (int i = 0; i < motivoDesalojo->parametrosLength; i++) 
        agregarAPaquete (paquete, (void *)motivoDesalojo->parametros[i], (strlen (motivoDesalojo->parametros[i]) + 1) * sizeof(char));

}

void agregarInstruccionesAPaqueteBeta(t_paquete* paquete, t_list* instrucciones, t_contexto *contexto) {
    contexto->instruccionesLength = list_size(instrucciones);

    // Primero enviamos la cantidad de elementos
    agregarAPaquete(paquete, &contexto->instruccionesLength, sizeof(uint32_t));
    

    for (uint32_t i = 0; i < contexto->instruccionesLength; i++) {
        char *instruccion = list_get(instrucciones, i);
        uint32_t instruccion_length = strlen(instruccion) + 1;

        // Enviar el tamaño de la instrucción
        agregarAPaquete(paquete, &instruccion_length, sizeof(uint32_t));
        log_info(logger,"instruccion %s", instruccion);
        // Enviar la instrucción
        agregarAPaquete(paquete, instruccion, instruccion_length);
    }
}

void agregarInstruccionesAPaquete(t_paquete* paquete, t_list* instrucciones){

    contextoEjecucion->instruccionesLength = list_size(instrucciones);
    log_info(logger, "ACA LLEGO PAPITOOOOOOOOOOOOOOOOOOOOO");
    agregarAPaquete(paquete, &contextoEjecucion->instruccionesLength, sizeof(uint32_t)); //primero envio la cantidad de elementos
    uint32_t i;
    for(i=0;i<contextoEjecucion->instruccionesLength;i++)
        agregarAPaquete (paquete, list_get(instrucciones, i), sizeof(char) * (strlen(list_get(instrucciones, i)) + 1 ));
    
}

void agregarRegistrosAPaquete(t_paquete* paquete, t_dictionary* registrosCPU){

    char name[3] = "AX", longName[4] = "EAX";

    for (int i = 0; i < 2; i++) {
        ssize_t tamanioActual = sizeof(char) * (4 * pow(2, i) + 1);
        for (int j = 0; j < 4; j++) {
            char* registroConCaracterTerminador = (char*) dictionary_get(registrosCPU, (i) ? longName : name); 
            //log_info(logger, "Registro %s: %s", (i) ? longName : name, registroConCaracterTerminador);
            agregarAPaquete(paquete, (void*) registroConCaracterTerminador, tamanioActual);
            name[0]++, longName[1]++;
        }
        longName [1] = 'A', longName [0] = (i == 1) ? 'R' : 'E';
    }

}
//Ver
void deserializarTablaDePaginas (void * buffer, int * desplazamiento) {
    list_clean_and_destroy_elements (contextoEjecucion->tablaDePaginas, free);
    // Desplazamiento: tamaño de la lista de paginas.
    memcpy(&(contextoEjecucion->tablaDePaginasSize), buffer + (* desplazamiento), sizeof(uint32_t));
    (* desplazamiento) += sizeof(uint32_t);
log_info(logger, "Tamaño de la tabla de paginas: %d", contextoEjecucion->tablaDePaginasSize);

contextoEjecucion->tablaDePaginasSize = 1; //TODO: FIX
    for (uint32_t i = 0; i < contextoEjecucion->tablaDePaginasSize; i++) { // SE QUEDA TRABADO EN ESTE FOR

        t_pagina* pagina = deserializarPagina(buffer, desplazamiento);
        list_add (contextoEjecucion->tablaDePaginas, pagina);
    }
log_info(logger, "*** salgo del for deserializarTablaDePaginas");
    (* desplazamiento) += sizeof(int);

}

//Ver
t_pagina* deserializarPagina(void* buffer, int* desplazamiento){
    log_info(logger, "Deserializando pagina...");
    t_pagina* pagina = malloc(sizeof(t_pagina));
    int tamanio;
    // id
    memcpy (&tamanio, buffer + (* desplazamiento), sizeof (int));
    (* desplazamiento) += sizeof (int);
    memcpy (&(pagina->idPagina), buffer + (* desplazamiento), sizeof(tamanio));
    (* desplazamiento) += sizeof (uint32_t);
    //id marco
    memcpy (&tamanio, buffer + (* desplazamiento), sizeof (int));
    (* desplazamiento) += sizeof (int);
    memcpy (&(pagina->idFrame), buffer + (* desplazamiento), sizeof(tamanio));
    (* desplazamiento) += sizeof (uint32_t);
    //bit
    memcpy (&tamanio, buffer + (* desplazamiento), sizeof (int));
    (* desplazamiento) += sizeof (int);
    memcpy (&(pagina->bitDeValidez), buffer + (* desplazamiento), sizeof(tamanio));
    (* desplazamiento) += sizeof (uint32_t);

    return pagina;
}