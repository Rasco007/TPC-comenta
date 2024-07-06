#include <cicloDeInstruccion/cicloDeInstruccion.h>

char *listaComandos[] = {
    [SET] = "SET",
    [MOV_IN] = "MOV_IN",
    [MOV_OUT] = "MOV_OUT",
    [SUM]="SUM",
    [SUB]="SUB",
    [JNZ]="JNZ",
    [RESIZE]="RESIZE",
    [COPY_STRING]="COPY_STRING",
    [IO_GEN_SLEEP]="IO_GEN_SLEEP",
    [IO_STDIN_READ]="IO_STDIN_READ",
    [IO_STDOUT_WRITE]="IO_STDOUT_WRITE",
    [IO_FS_CREATE]="IO_FS_CREATE",
    [IO_FS_DELETE]="IO_FS_DELETE",
    [IO_FS_TRUNCATE]="IO_FS_TRUNCATE",
    [IO_FS_WRITE]="IO_FS_WRITE",
    [IO_FS_READ]="IO_FS_READ",
    [WAIT] = "WAIT",
    [SIGNAL] = "SIGNAL",
    [EXIT] = "EXIT"
};

char* instruccionAEjecutar; 
char** elementosInstruccion; 
int instruccionActual; 
int cantParametros;
int tiempoEspera;
int nroSegmento;

t_temporal* rafagaCPU; 
int64_t rafagaCPUEjecutada; 

void cicloDeInstruccion(){
    fetch();//busca la próxima instruccion a ejecutar. Lista en pcb
    decode();//interpreta que instruccion va a ejecutar y si requiere traduccion logica o fisica
    execute();//ejecuta la instruccion
    check_interrupt(); //control de quantum
    liberarMemoria();
}
void solicitarInstruccion(int pid, int indice, int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = 2*sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	
	memcpy(paquete->buffer->stream, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &indice, sizeof(int));
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

// ------- Funciones del ciclo ------- //
void fetch() { 
    log_info(logger, "inicio fetch");
    //Mando la linea que voy a leer de la lista de instrucciones de memoria
    int pid=contextoEjecucion->pid;
    int numInstruccionABuscar = contextoEjecucion->programCounter;
    /*t_paquete* paquete = crearPaquete();
    agregarAPaquete(paquete, &numInstruccionABuscar, sizeof(int)); //PC
    agregarAPaquete(paquete, &pid, sizeof(int));//PID
    enviarPaquete(paquete, conexionAMemoria);*/
    solicitarInstruccion(pid, numInstruccionABuscar, conexionAMemoria);
    //Recibo la instruccion
    int peticion = recibirOperacion(conexionAMemoria);
    //int x=1;
    //while(x==1){
	switch (peticion) {
		case MENSAJE:
			instruccionAEjecutar=recibirMensaje(conexionAMemoria);
            log_info(logger, "Instruccion recibida: %s", instruccionAEjecutar);
			break;
		default:
            log_warning(logger,"Operacion desconocida.");
			break;
	}
	//}
    contextoEjecucion->programCounter += 1;
}

void decode(){
    // Eliminar el salto de línea al final de instruccionAEjecutar
    remove_newline(instruccionAEjecutar);

    log_info(logger, "inicio decode con instruccionAEjecutar: %s", instruccionAEjecutar);
    elementosInstruccion = string_n_split(instruccionAEjecutar, 4, " ");
    log_info(logger, "instruccion a ejecutar: %s", elementosInstruccion[0]); 
    cantParametros = string_array_size(elementosInstruccion) - 1; 
    log_info(logger, "cantParametros: %d", cantParametros);
    instruccionActual = buscar(elementosInstruccion[0], listaComandos); 
    log_info(logger, "instruccion Actual: %d", instruccionActual);
    
}

// Función para eliminar el salto de línea al final de la cadena
void remove_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int buscar(char *elemento, char **lista) {
    int i = 0;
    
    while (i <= string_array_size (lista)) {
        if (i < string_array_size (lista))
            if (!strcmp (elemento, lista[i]))
                return i;
        i++;
    }
    return (i > string_array_size(lista)) ? -1 : i;
}
 
void check_interrupt(){
    if(contextoEjecucion->algoritmo != FIFO){
        log_info(logger, "inicio check_interrupt");
        int64_t quantum=contextoEjecucion->quantum;
        t_temporal* tiempoDeUsoCPU = contextoEjecucion->tiempoDeUsoCPU;
        //Si el cronometro marca un tiempo superior al quantum, desalojo el proceso
        log_info(logger, temporal_gettime(tiempoDeUsoCPU)>=quantum ? "entro al if porque es true" : "NO entro al if porque es false");
        if(temporal_gettime(tiempoDeUsoCPU)>=quantum){
            destruirTemporizador(rafagaCPU); // ACA TIRA SEGMENTATION FAULT
            modificarMotivoDesalojo (FIN_DE_QUANTUM, 0, "", "", "", "", "");
            enviarContextoBeta(socketClienteInterrupt, contextoEjecucion);
        }
    }
}

// Instrucciones
void io_fs_delete(char* interfaz,char* nombreArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_DELETE, 2, interfaz, nombreArchivo, "", "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void io_stdout_write(char* interfaz, char* registroDireccion, char* RegistroTamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_STDOUT_WRITE, 3, interfaz, registroDireccion, RegistroTamanio, "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void io_fs_truncate(char* interfaz, char* nombreArchivo, char* RegistroTamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_TRUNCATE, 3, interfaz, nombreArchivo, RegistroTamanio, "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void io_fs_create(char* interfaz, char* nombreArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_CREATE, 2, interfaz, nombreArchivo, "", "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void io_fs_write(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_WRITE, 5, interfaz, nombreArchivo, registroDireccion, registroTamanio, registroPunteroArchivo);
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void io_fs_read(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_READ, 5, interfaz, nombreArchivo, registroDireccion, registroTamanio, registroPunteroArchivo);
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void io_stdin_read(char* interfaz, char* registroDireccion, char* registroTamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_STDIN_READ, 3, interfaz, registroDireccion, registroTamanio, "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void copy_string(char* tamanio){
    //Copiar contenido de SI a DI
    contextoEjecucion->DI=contextoEjecucion->DI; //Ver
}

void resize(char* tamanio){
    t_paquete* paquete = crearPaquete();
    paquete->codigo_operacion = RESIZE;
    agregarAPaquete(paquete, tamanio, sizeof(tamanio));
    agregarAPaquete(paquete, &contextoEjecucion->pid, sizeof(uint32_t));
    enviarPaquete(paquete, conexionAMemoria);

}

void set_c(char* registro, char* valor){ 
    log_info(logger, "inicio set_c con registro: %s y valor: %s", registro, valor);
    //tiempoEspera = obtenerTiempoEspera();
    //log_info(logger, "tiempoEspera: %d", tiempoEspera);
    //usleep(10 * 1000); 
    //dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro, free); 
    dictionary_put(contextoEjecucion->registrosCPU, registro, string_duplicate(valor)); //TODO: FIX
     // Log the first value of registrosCPU
    //char* puse = dictionary_get(contextoEjecucion->registrosCPU, registro);
    //log_info(logger, "te lo muestro en fin set_c: %s", puse);
}

void sum_c(char* registro_destino, char* registro_origen){ 
    // Verificar que los registros existen en el diccionario
    char* valorDestino = dictionary_get(contextoEjecucion->registrosCPU, registro_destino);
    char* valorOrigen = dictionary_get(contextoEjecucion->registrosCPU, registro_origen);
    if (!valorDestino || !valorOrigen) {
        log_error(logger, "Registro no encontrado: %s o %s", registro_destino, registro_origen);
        return;
    }
    int primerValor = atoi(valorDestino);
    int segundoValor = atoi(valorOrigen);
    int resultado = primerValor + segundoValor;
    char* resultadoStr = malloc(sizeof(char) * 10);
    sprintf(resultadoStr,"%d", resultado);
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro_destino, free);
    dictionary_put(contextoEjecucion->registrosCPU, registro_destino, resultadoStr);
    log_info(logger, "fin sum_c");
}

void sub_c(char* registro_destino, char* registro_origen){ 
    // Verificar que los registros existen en el diccionario
    char* valorDestino = dictionary_get(contextoEjecucion->registrosCPU, registro_destino);
    char* valorOrigen = dictionary_get(contextoEjecucion->registrosCPU, registro_origen);
    if (!valorDestino || !valorOrigen) {
        log_error(logger, "Registro no encontrado: %s o %s", registro_destino, registro_origen);
        return;
    }
    int primerValor = atoi(valorDestino);
    int segundoValor = atoi(valorOrigen);
    int resultado = primerValor - segundoValor;
    char* resultadoStr = malloc(sizeof(char) * 10);
    sprintf(resultadoStr, "%d", resultado);
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro_destino, free);
    dictionary_put(contextoEjecucion->registrosCPU, registro_destino, resultadoStr);
}


void jnz(char* registro, char* instruccion){ 
    log_info(logger,"En jnz con: %s y %s", registro, instruccion);
    // Obtener el valor del registro desde el diccionario
    char* valorRegistro = dictionary_get(contextoEjecucion->registrosCPU, registro);
    if (!valorRegistro) {
        log_error(logger, "Registro no encontrado: %s", registro);
        return;
    }

    // Convertir el valor del registro a un entero
    int valor = atoi(valorRegistro);
    
    // Si el valor no es 0, actualizar el contador de programa (programCounter)
    if (valor != 0) {
        log_info(logger, "contextoEjecucion->programCounter: %d", contextoEjecucion->programCounter);
        log_info(logger, "Valor de registro: %d", atoi(instruccion));

        contextoEjecucion->programCounter = atoi(instruccion);
    }
}

void io_gen_sleep(char* interfaz, char* unidades_trabajo){ 
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_GEN_SLEEP, 2, interfaz, unidades_trabajo, "", "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void wait_c(char* recurso){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (WAIT, 1, recurso, "", "", "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void signal_c(char* recurso){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (SIGNAL, 1, recurso, "", "", "", "");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion);
}

void exit_c () {
    //log_info(logger, "Antes destruirTemporizador rafagaCPU: %p", rafagaCPU);
    //destruirTemporizador(rafagaCPU); TODO: DESTRUIR
    //log_info(logger, "Pasa destruirTemporizador");
    char * terminado = string_duplicate ("SUCCESS");
    modificarMotivoDesalojo (EXIT, 1, terminado, "", "", "", "");
    log_info(logger, "Pasa modificarMotivoDesalojo");
    enviarContextoBeta(socketClienteDispatch, contextoEjecucion); 
    free (terminado);
    log_info(logger, "fin exit_c");
}


void mov_in(char* registro, char* direccionLogica){

    char* valorAInsertar;
    uint32_t tamRegistro = (uint32_t)obtenerTamanioReg(registro);
    uint32_t dirFisica = UINT32_MAX;
    dirFisica = 0; //mmu(direccionLogica, tamRegistro); //TODO: pasar TLB? 

    log_info(logger, "Direccion fisica: %d", dirFisica);

    if(dirFisica!=UINT32_MAX){
        t_paquete* peticion = crearPaquete();
        peticion->codigo_operacion = READ;
        agregarAPaquete(peticion,&contextoEjecucion->pid, sizeof(uint32_t));
        agregarAPaquete(peticion,&dirFisica, sizeof(uint32_t));
        agregarAPaquete(peticion,&tamRegistro,sizeof(uint32_t));
        enviarPaquete(peticion, conexionAMemoria);    
        eliminarPaquete (peticion);

        recibirOperacion(conexionAMemoria);
        valorAInsertar = recibirMensaje(conexionAMemoria);

        dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro, free); 
        dictionary_put(contextoEjecucion->registrosCPU, registro, string_duplicate(valorAInsertar));
        
        log_info(logger, "PID: <%d> - Accion: <%s> - Segmento: <%d> - Direccion Fisica: <%d> - Valor: <%s>", contextoEjecucion->pid, "LEER", nroSegmento, dirFisica, valorAInsertar);
        free (valorAInsertar);
    }else {
        log_info(logger, "Error: Dirección física inválida\n");
    }
};

void mov_out(char* direccionLogica, char* registro){

    void * valor = dictionary_get(contextoEjecucion->registrosCPU, registro);
    int tamRegistro = obtenerTamanioReg(registro);

    uint32_t dirFisica = UINT32_MAX;
    dirFisica = 0; //mmu(direccionLogica, tamRegistro); //TODO: pasar TLB? 

    if(dirFisica != UINT32_MAX){    
    t_paquete* peticion = crearPaquete();
    peticion->codigo_operacion = WRITE;

    agregarAPaquete(peticion, &(contextoEjecucion->pid), sizeof(uint32_t));
    agregarAPaquete(peticion, &(dirFisica), sizeof(int));
    agregarAPaquete(peticion, valor, sizeof(char) * tamRegistro + 1); 

    enviarPaquete(peticion, conexionAMemoria);
    eliminarPaquete (peticion);

    recibirOperacion(conexionAMemoria);
    char * respuesta = recibirMensaje(conexionAMemoria);
    free (respuesta);

    log_info(logger, "PID: <%d> - Accion: <%s> - Segmento: <%d> - Direccion Fisica: <%d> - Valor: <%s>", contextoEjecucion->pid, "WRITE", nroSegmento, dirFisica, (char *)valor);
    }
};

// Funciones grales

void destruirTemporizador (t_temporal * temporizador) {
    temporal_stop(temporizador); 
}

void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3, char * parm4, char * parm5) {
    char * (parametros[5]) = { parm1, parm2, parm3, parm4, parm5};
    contextoEjecucion->motivoDesalojo->motivo = comando;
    log_info(logger, "numero de parametros en motivo de EXIT %d", numParametros);
    contextoEjecucion->motivoDesalojo->parametrosLength = numParametros;
    for (int i = 0; i < numParametros; i++){
     
        contextoEjecucion->motivoDesalojo->parametros[i] = string_duplicate(parametros[i]);
    
    log_info(logger, "parametro EXIT %s" , contextoEjecucion->motivoDesalojo->parametros[i] );
    }
}

void liberarMemoria() {
    for (int i = 0; i <= cantParametros; i++) free(elementosInstruccion[i]);
    free(elementosInstruccion);
    log_info(logger,"Memoria liberada!");
}

char* recibirValor(int socket) {
    char* valor;
    int tamanio; 
	int size, desplazamiento = 0;
	void * buffer;

	buffer = recibirBuffer(socket, &size);

    memcpy(&(tamanio), buffer, sizeof(int32_t));
    desplazamiento += sizeof(int32_t);

    memcpy(&(valor),buffer+desplazamiento,sizeof(char)*tamanio); 
    desplazamiento+=sizeof(char)*tamanio; 

	free(buffer);

    return valor;
}

int obtenerTamanioReg(char* registro){
    if(string_starts_with(registro, "E")) return 8;
    else if(string_starts_with(registro, "R")) return 16;
    else return 4;
}

void execute() {
    log_info(logger, "inicio execute");

    switch(cantParametros) {
        case 0:
            log_info(logger, "case 0 PID: <%d> - Ejecutando: <%s> ", contextoEjecucion->pid, listaComandos[instruccionActual]);
            break;
        case 1:
            log_info(logger, "case 1 PID: <%d> - Ejecutando: <%s> -  <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1]);
            break;
        case 2:   
            log_info(logger, "case 2 PID: <%d> - Ejecutando: <%s> - <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case 3:
            log_info(logger, "case 3 PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3]);
            break; 
        case 4:
            log_info(logger, "case 4 PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4]);
            break;
        case 5:
            log_info(logger, "case 5 PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4], elementosInstruccion[5]);
            break;
    }
    log_info(logger, "instruccionActual: %d", instruccionActual);
    switch(instruccionActual){//TODO: Completar con instrucciones restantes
        case SET: 
            set_c(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case SUM:
            sum_c(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case SUB:
            sub_c(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case JNZ:
            jnz(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case IO_GEN_SLEEP:
            io_gen_sleep(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case MOV_IN:
            mov_in(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case MOV_OUT:
            mov_out(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case WAIT:
            wait_c(elementosInstruccion[1]);
            break;
        case SIGNAL:
            signal_c(elementosInstruccion[1]);
            break;
        case EXIT:
            exit_c();
            break;
        case IO_STDIN_READ:
            io_stdin_read(elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3]);
            break;
        case IO_STDOUT_WRITE:
            io_stdout_write(elementosInstruccion[1],elementosInstruccion[2], elementosInstruccion[3]);
            break;
        case IO_FS_CREATE:
            io_fs_create(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case IO_FS_DELETE:
            io_fs_delete(elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case IO_FS_TRUNCATE:
            io_fs_truncate(elementosInstruccion[1], elementosInstruccion[2],elementosInstruccion[3]);
            break;
        case IO_FS_WRITE:
            io_fs_write(elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4], elementosInstruccion[5]);
            break;
        case IO_FS_READ:
            io_fs_read(elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3], elementosInstruccion[4], elementosInstruccion[5]);
            break;
        case RESIZE:
            resize(elementosInstruccion[1]);
            break;
        case COPY_STRING:
            copy_string(elementosInstruccion[1]);
            break;
        default:
            break;
    }
}