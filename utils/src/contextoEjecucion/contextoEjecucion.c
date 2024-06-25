#include <contextoEjecucion/contextoEjecucion.h>
t_contexto * contextoEjecucion = NULL;
t_dictionary *crearDiccionarioDeRegistros2();

// MANEJO DE CONTEXTO
void enviarContextoActualizado(int socket){ 
    t_paquete * paquete = crearPaquete();
    
    paquete->codigo_operacion = CONTEXTOEJECUCION;

    log_info(logger, "Hardcodeo");

    contextoEjecucion->instrucciones = list_create();
	contextoEjecucion->instruccionesLength = 0;
	contextoEjecucion->pid = 1;
	contextoEjecucion->programCounter = 0;
	contextoEjecucion->registrosCPU = crearDiccionarioDeRegistros2();
	contextoEjecucion->tablaDePaginas = list_create();
	contextoEjecucion->tablaDePaginasSize = 0;
    //contextoEjecucion->rafagaCPUEjecutada = 0;
    contextoEjecucion->motivoDesalojo = (t_motivoDeDesalojo *)malloc(sizeof(t_motivoDeDesalojo));
    contextoEjecucion->motivoDesalojo->parametros[0] = "";
    contextoEjecucion->motivoDesalojo->parametros[1] = "";
    contextoEjecucion->motivoDesalojo->parametros[2] = "";
    contextoEjecucion->motivoDesalojo->parametrosLength = 0;
    contextoEjecucion->motivoDesalojo->motivo = 0;

    
   
    agregarAPaquete (paquete,(void *)&contextoEjecucion->pid, sizeof(contextoEjecucion->pid));
    agregarAPaquete (paquete,(void *)&contextoEjecucion->programCounter, sizeof(contextoEjecucion->programCounter));
    
    agregarInstruccionesAPaquete (paquete, contextoEjecucion->instrucciones);
    log_info(logger, "Instrucciones agregadas al paquete");

    agregarRegistrosAPaquete(paquete, contextoEjecucion->registrosCPU);
    log_info(logger, "Registros agregados al paquete");
    
    agregarTablaDePaginasAPaquete(paquete);
    log_info(logger, "Tabla de paginas agregada al paquete");

    agregarMotivoAPaquete(paquete, contextoEjecucion->motivoDesalojo);
    log_info(logger, "Motivo de desalojo agregado al paquete");

    agregarAPaquete(paquete, (void *)&contextoEjecucion->tiempoDeUsoCPU, sizeof(contextoEjecucion->tiempoDeUsoCPU));
    log_info(logger, "Rafaga de CPU agregada al paquete");

    enviarPaquete(paquete, socket);
    log_info(logger, "Procede a eliminar paquete");
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

    // Desplazamiento: Tamaño de la rafaga de CPU Ejecutada.
    desplazamiento += sizeof (int);
    memcpy(&(contextoEjecucion->tiempoDeUsoCPU), buffer + desplazamiento, sizeof (uint64_t));
		
	free(buffer);

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
    //log_info(logger, "Tamaño de la tabla de paginas: %d", contextoEjecucion->tablaDePaginasSize);

    for (uint32_t i = 0; i < contextoEjecucion->tablaDePaginasSize; i++) { // SE QUEDA TRABADO EN ESTE FOR

        t_pagina* pagina = deserializarPagina(buffer, desplazamiento);
        list_add (contextoEjecucion->tablaDePaginas, pagina);
    }

    (* desplazamiento) += sizeof(int);

}

//Ver
t_pagina* deserializarPagina(void* buffer, int* desplazamiento){
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