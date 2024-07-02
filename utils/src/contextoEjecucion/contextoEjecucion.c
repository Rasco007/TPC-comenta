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

    for (uint32_t i = 0; i < list_size(contexto->instrucciones); i++) {
        char* instruccion = list_get(contexto->instrucciones, i);
        paquete->buffer->size += sizeof(uint32_t) + strlen(instruccion) + 1; // Tamaño de la instrucción + la instrucción
    }

     // Calcular el tamaño de los registros
    paquete->buffer->size  += 4 * (4 + 8); // AX, BX, CX, DX (4 bytes cada uno) + EAX, EBX, ECX, EDX (8 bytes cada uno)
    
    paquete->buffer->stream = malloc(paquete->buffer->size);

    // Serializar los datos en el buffer
    int desplazamiento = 0;
    //agregar a paquete
    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->pid), sizeof(contexto->pid));
    desplazamiento += sizeof(contexto->pid);

    memcpy(paquete->buffer->stream + desplazamiento, &(contexto->programCounter), sizeof(contexto->programCounter));
    desplazamiento += sizeof(contexto->programCounter);
 
   
    log_info(logger, "length %d",contexto->instruccionesLength);
   memcpy(paquete->buffer->stream + desplazamiento, &(contexto->instruccionesLength), sizeof(contexto->instruccionesLength));
    desplazamiento += sizeof(contexto->instruccionesLength);

    // Serializar las instrucciones
    for (uint32_t i = 0; i < contexto->instruccionesLength; i++) {
        char* instruccion = list_get(contexto->instrucciones, i);
        uint32_t instruccion_length = strlen(instruccion) + 1;

        memcpy(paquete->buffer->stream + desplazamiento, &instruccion_length, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
        log_info(logger,"instruccion %s", instruccion);
        memcpy(paquete->buffer->stream + desplazamiento, instruccion, instruccion_length);
        desplazamiento += instruccion_length;
    }

    // Serializar los registros
    char* registros[] = {"AX", "BX", "CX", "DX", "EAX", "EBX", "ECX", "EDX"};
    for (int i = 0; i < 8; i++) {
        char* registro = dictionary_get(contextoEjecucion->registrosCPU, registros[i]);
        int registro_length = (i < 4) ? 4 : 8;
        memcpy(paquete->buffer->stream  + desplazamiento, registro, registro_length);
        desplazamiento += registro_length;
         log_info(logger, "Serializando registro %s: %s", registros[i], registro);
    }


    // Calcular el tamaño total del paquete a enviar
    int bytes = sizeof(op_code) + sizeof(int) + paquete->buffer->size;
    
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
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }

    t_contexto *contextoEjecucionBeta = malloc(sizeof(t_contexto));
    contextoEjecucionBeta->instrucciones = list_create();
     contextoEjecucionBeta->registrosCPU = dictionary_create();

    int desplazamiento = sizeof(op_code);

    memcpy(&(contextoEjecucionBeta->pid), buffer + desplazamiento, sizeof(contextoEjecucionBeta->pid));
    desplazamiento += sizeof(contextoEjecucionBeta->pid);

    memcpy(&(contextoEjecucionBeta->programCounter), buffer + desplazamiento, sizeof(contextoEjecucionBeta->programCounter));
    desplazamiento += sizeof(contextoEjecucionBeta->programCounter);

    log_info(logger, "llega hasta aqui");

    memcpy(&(contextoEjecucionBeta->instruccionesLength), buffer + desplazamiento, sizeof(contextoEjecucionBeta->instruccionesLength));
    desplazamiento += sizeof(contextoEjecucionBeta->instruccionesLength);

    //deserealizo las instrucciones
    for (uint32_t i = 0; i < contextoEjecucionBeta->instruccionesLength; i++) {
        uint32_t instruccion_length;
        memcpy(&instruccion_length, buffer + desplazamiento, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
 log_info(logger,"instruccion Length %u", instruccion_length);
        char* instruccion = malloc(instruccion_length);
        memcpy(instruccion, buffer + desplazamiento, instruccion_length);
        desplazamiento += instruccion_length;

        // Asegúrate de terminar la cadena con '\0'
        instruccion[instruccion_length - 1] = '\0';

        list_add(contextoEjecucionBeta->instrucciones, instruccion);
        log_info(logger, "instruccion: %s", instruccion);
    }

    printf("Recibido PID: %u PC: %d \n", contextoEjecucionBeta->pid, contextoEjecucionBeta->programCounter);
    

    //deserealizo los registros 
    char* registro;
    char nombreRegistro[4];

    // AX, BX, CX, DX (4 bytes cada uno)
    for (char nombre = 'A'; nombre <= 'D'; nombre++) {
        registro = malloc(5); // 4 bytes + terminador nulo
        memcpy(registro, buffer + desplazamiento, 4);
        registro[4] = '\0';
        desplazamiento += 4;

        snprintf(nombreRegistro, 3, "%cX", nombre);
        dictionary_put(contextoEjecucionBeta->registrosCPU, nombreRegistro, registro);
        log_info(logger, "Registro %s: %s", nombreRegistro, registro);
    }

    // EAX, EBX, ECX, EDX (8 bytes cada uno)
    for (char nombre = 'A'; nombre <= 'D'; nombre++) {
        registro = malloc(9); // 8 bytes + terminador nulo
        memcpy(registro, buffer + desplazamiento, 8);
        registro[8] = '\0';
        desplazamiento += 8;

        snprintf(nombreRegistro, 4, "E%cX", nombre);
        dictionary_put(contextoEjecucionBeta->registrosCPU, nombreRegistro, registro);
        log_info(logger, "Registro %s: %s", nombreRegistro, registro);
    }

    free(contextoEjecucionBeta);
    
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



void recibirContextoActualizado (int socket) {
    log_info(logger, "------ Recibiendo contexto actualizado...");
    if (contextoEjecucion != NULL) destroyContextoUnico ();
	iniciarContexto ();
	int size, desplazamiento = 0;
	void * buffer;

	buffer = recibirBuffer(socket, &size);
    // Desplazamiento: Tamaño de PID, PID, y tamaño de programCounter.
    desplazamiento += sizeof(int);
    memcpy(&(contextoEjecucion->pid), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(contextoEjecucion->pid) + sizeof(int);

    // Desplazamiento: programCounter y tamaño de instruccionesLength.
    memcpy(&(contextoEjecucion->programCounter), buffer+desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(contextoEjecucion->programCounter) + sizeof(int);
    deserializarInstrucciones (buffer, &desplazamiento);
    
    deserializarRegistros (buffer, &desplazamiento);

    deserializarTablaDePaginas(buffer, &desplazamiento);
    
    deserializarMotivoDesalojo (buffer, &desplazamiento);
    log_info(logger, "------ deserializarMotivoDesalojo");

    // Desplazamiento: Tamaño de la rafaga de CPU Ejecutada.
    desplazamiento += sizeof (int);
    memcpy(&(contextoEjecucion->tiempoDeUsoCPU), buffer + desplazamiento, sizeof (uint64_t));
		
	free(buffer);

}
void deserializarInstruccionesBeta(void *buffer, int *desplazamiento, t_contexto *contextoEjecucionBeta) {
    uint32_t instruccionesLength;
    memcpy(&instruccionesLength, buffer + (*desplazamiento), sizeof(uint32_t));
    (*desplazamiento) += sizeof(uint32_t);

    list_clean_and_destroy_elements(contextoEjecucionBeta->instrucciones, free);

    for (uint32_t i = 0; i < instruccionesLength; i++) {
        uint32_t instruccion_length;
        memcpy(&instruccion_length, buffer + (*desplazamiento), sizeof(uint32_t));
        (*desplazamiento) += sizeof(uint32_t);

        char *instruccion = malloc(instruccion_length);
        memcpy(instruccion, buffer + (*desplazamiento), instruccion_length);
        (*desplazamiento) += instruccion_length;

        list_add(contextoEjecucionBeta->instrucciones, instruccion);
    }
}

void deserializarInstrucciones (void * buffer, int * desplazamiento) {

    int tamanio;
    list_clean_and_destroy_elements (contextoEjecucion->instrucciones, free);
    // Desplazamiento: instruccionesLength.
    memcpy(&(contextoEjecucion->instruccionesLength), buffer + (* desplazamiento), sizeof(uint32_t));
    (* desplazamiento) += sizeof(uint32_t);
    
    for (uint32_t i = 0; i < contextoEjecucion->instruccionesLength; i++) {

        // Desplazamiento: Tamaño de la linea de instruccion.
        memcpy (&tamanio, buffer + (* desplazamiento), sizeof (int));
        (* desplazamiento) += sizeof (int);
        char * valor = malloc (tamanio);

        //Desplazamiento: Linea de instruccion.
        memcpy(valor, buffer + (* desplazamiento), tamanio);
        //debug ("%s", valor);
        (* desplazamiento) += tamanio;
        list_add (contextoEjecucion->instrucciones, string_duplicate (valor));
        free (valor);
    }

    (* desplazamiento) += sizeof(int);

}

void deserializarRegistros (void * buffer, int * desplazamiento) {
    dictionary_clean_and_destroy_elements (contextoEjecucion->registrosCPU, free);

    char *temp, name [3] = "AX", longName [4] = "EAX";
    
    for (int i = 0; i < 3; i++) {
        ssize_t tamanioActual = sizeof(char) * (4 * pow(2, i) + 1);
        for (int j = 0; j < 4; j++) {
            temp = malloc (tamanioActual);

            // Desplazamiento: Registro actual y tamaño del proximo registro. 
            // (Para el ultimo registro pasa a ser el tamaño del comando de desalojo)
            memcpy (temp, buffer + (* desplazamiento), tamanioActual);
            (* desplazamiento) += tamanioActual + sizeof (int);

            dictionary_put (contextoEjecucion->registrosCPU, (i) ? longName : name, temp);
            name [0]++, longName [1]++;
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


void deserializarMotivoDesalojo (void * buffer, int * desplazamiento) {
    // Desplazamiento: Comando de desalojo y tamaño de parametrosLength.
    memcpy (&(contextoEjecucion->motivoDesalojo->motivo), buffer + (* desplazamiento), sizeof (t_comando));
    (* desplazamiento) += sizeof (t_comando) + sizeof (int);

    // Desplazamiento: parametrosLength.
    memcpy (&(contextoEjecucion->motivoDesalojo->parametrosLength), buffer + (* desplazamiento), sizeof (uint32_t));
    (* desplazamiento) += sizeof (contextoEjecucion->motivoDesalojo->parametrosLength);
log_info(logger, "---- antes del for, parametrosLength: %d", contextoEjecucion->motivoDesalojo->parametrosLength);
contextoEjecucion->motivoDesalojo->parametrosLength = 4; //TODO: FIX
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) {
        int tamanioParametro;

        // Desplazamiento: Tamaño de Parametro.
        memcpy (&tamanioParametro, buffer + (* desplazamiento), sizeof (int));
        (* desplazamiento) += sizeof(int);

        // Desplazamiento: Parametro.
        char *temp = malloc (sizeof(char) * tamanioParametro);
        memcpy(temp, buffer + (* desplazamiento), sizeof (char) * tamanioParametro);
        contextoEjecucion->motivoDesalojo->parametros[i] = temp;
        (* desplazamiento) += tamanioParametro;
    }
log_info(logger, "---- salgo del for");

}

void iniciarContexto(){

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
    contextoEjecucion->motivoDesalojo->parametros[0] = "";
    contextoEjecucion->motivoDesalojo->parametros[1] = "";
    contextoEjecucion->motivoDesalojo->parametros[2] = "";
    contextoEjecucion->motivoDesalojo->parametrosLength = 0;
    contextoEjecucion->motivoDesalojo->motivo = 0;
	
}

void destroyContexto() {
    list_destroy_and_destroy_elements(contextoEjecucion->instrucciones, free);
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    list_destroy_and_destroy_elements(contextoEjecucion->tablaDePaginas, free);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) 
        if (strcmp(contextoEjecucion->motivoDesalojo->parametros[i], "")) free(contextoEjecucion->motivoDesalojo->parametros[i]);
    free(contextoEjecucion->motivoDesalojo);
    free(contextoEjecucion);
    contextoEjecucion = NULL;
}

void destroyContextoUnico () {
    list_destroy(contextoEjecucion->instrucciones);
    dictionary_destroy_and_destroy_elements(contextoEjecucion->registrosCPU, free);
    list_destroy(contextoEjecucion->tablaDePaginas);
    for (int i = 0; i < contextoEjecucion->motivoDesalojo->parametrosLength; i++) 
        if (strcmp(contextoEjecucion->motivoDesalojo->parametros[i], "")) free(contextoEjecucion->motivoDesalojo->parametros[i]);
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