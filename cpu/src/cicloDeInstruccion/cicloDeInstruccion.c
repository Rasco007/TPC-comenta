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

void cicloDeInstruccion(int socketClienteInterrupt, int socketClienteDispatch){
    fetch();//busca la próxima instruccion a ejecutar. Lista en pcb
    decode();//interpreta que instruccion va a ejecutar y si requiere traduccion logica o fisica
    execute();//ejecuta la instruccion
    check_interrupt(socketClienteInterrupt,socketClienteDispatch); //revisa si hay interrupciones 
    liberarMemoria();
}

// ------- Funciones del ciclo ------- //
void fetch() { 
    int numInstruccionABuscar = contextoEjecucion->programCounter;
    instruccionAEjecutar = list_get(contextoEjecucion->instrucciones,numInstruccionABuscar); 
    contextoEjecucion->programCounter += 1;
}

void decode(){
    elementosInstruccion = string_n_split(instruccionAEjecutar, 4, " ");
    cantParametros = string_array_size(elementosInstruccion) - 1;
    instruccionActual = buscar(elementosInstruccion[0], listaComandos);
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
 
void check_interrupt(int socketClienteInterrupt, int socketClienteDispatch){
    if (recv(socketClienteInterrupt, NULL, 0, MSG_PEEK) != -1) {
        //motivo de desalojo???
        enviarContextoActualizado(socketClienteDispatch);
    }
}

// Instrucciones
void io_fs_delete(char* interfaz,char* nombreArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_DELETE, 2, interfaz, nombreArchivo, "", "", "");
    enviarContextoActualizado(socketCliente);
}

void io_stdout_write(char* interfaz, char* registroDireccion, char* RegistroTamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_STDOUT_WRITE, 3, interfaz, registroDireccion, RegistroTamanio, "", "");
    enviarContextoActualizado(socketCliente);
}

void io_fs_truncate(char* interfaz, char* nombreArchivo, char* RegistroTamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_TRUNCATE, 3, interfaz, nombreArchivo, RegistroTamanio, "", "");
    enviarContextoActualizado(socketCliente);
}

void io_fs_create(char* interfaz, char* nombreArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_CREATE, 2, interfaz, nombreArchivo, "", "", "");
    enviarContextoActualizado(socketCliente);
}

void io_fs_write(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_WRITE, 5, interfaz, nombreArchivo, registroDireccion, registroTamanio, registroPunteroArchivo);
    enviarContextoActualizado(socketCliente);
}

void io_fs_read(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_FS_READ, 5, interfaz, nombreArchivo, registroDireccion, registroTamanio, registroPunteroArchivo);
    enviarContextoActualizado(socketCliente);
}

void io_stdin_read(char* interfaz, char* registroDireccion, char* registroTamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_STDIN_READ, 3, interfaz, registroDireccion, registroTamanio, "", "");
    enviarContextoActualizado(socketCliente);
}

void copy_string(char* tamanio){
    //Copiar contenido de SI a DI
    contextoEjecucion->DI=contextoEjecucion->DI; //Ver
}

void resize(char* tamanio){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (RESIZE, 1, tamanio, "", "", "", "");
    enviarContextoActualizado(socketCliente);
}

void set_c(char* registro, char* valor){ 
    tiempoEspera = obtenerTiempoEspera();
    usleep(tiempoEspera * 1000); 
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro, free); 
    dictionary_put(contextoEjecucion->registrosCPU, registro, string_duplicate(valor));
}

void sum_c(char* registro_destino, char* registro_origen){ 
    int valorDestino = atoi(dictionary_get(contextoEjecucion->registrosCPU, registro_destino));
    int valorOrigen = atoi(dictionary_get(contextoEjecucion->registrosCPU, registro_origen));
    int resultado = valorDestino + valorOrigen;
    char* resultadoStr = malloc(sizeof(char) * 10);
    sprintf(resultadoStr, "%d", resultado);
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro_destino, free);
    dictionary_put(contextoEjecucion->registrosCPU, registro_destino, resultadoStr);
}

void sub_c(char* registro_destino, char* registro_origen){ 
    int valorDestino = atoi(dictionary_get(contextoEjecucion->registrosCPU, registro_destino));
    int valorOrigen = atoi(dictionary_get(contextoEjecucion->registrosCPU, registro_origen));
    int resultado = valorDestino - valorOrigen;
    char* resultadoStr = malloc(sizeof(char) * 10);
    sprintf(resultadoStr, "%d", resultado);
    dictionary_remove_and_destroy(contextoEjecucion->registrosCPU, registro_destino, free);
    dictionary_put(contextoEjecucion->registrosCPU, registro_destino, resultadoStr);
}


void jnz(char* registro, char* instruccion){ 
    int valorRegistro = atoi(dictionary_get(contextoEjecucion->registrosCPU, registro));
    if (valorRegistro != 0) {
        contextoEjecucion->programCounter = atoi(instruccion);
    }
}

void io_gen_sleep(char* interfaz, char* unidades_trabajo){ 
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (IO_GEN_SLEEP, 2, interfaz, unidades_trabajo, "", "", "");
    enviarContextoActualizado(socketCliente);
}

void wait_c(char* recurso){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (WAIT, 1, recurso, "", "", "", "");
    enviarContextoActualizado(socketCliente);
}

void signal_c(char* recurso){
    destruirTemporizador(rafagaCPU);
    modificarMotivoDesalojo (SIGNAL, 1, recurso, "", "", "", "");
    enviarContextoActualizado(socketCliente);
}

void exit_c () {
    destruirTemporizador(rafagaCPU);
    char * terminado = string_duplicate ("SUCCESS");
    modificarMotivoDesalojo (EXIT, 1, terminado, "", "", "", "");
    enviarContextoActualizado(socketCliente);
    free (terminado);
}


void mov_in(char* registro, char* direccionLogica){

    char* valorAInsertar;
    uint32_t tamRegistro = (uint32_t)obtenerTamanioReg(registro);
    uint32_t dirFisica = UINT32_MAX;
    dirFisica = mmu(direccionLogica, tamRegistro);

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
    }
};

void mov_out(char* direccionLogica, char* registro){

    void * valor = dictionary_get(contextoEjecucion->registrosCPU, registro);
    int tamRegistro = obtenerTamanioReg(registro);

    uint32_t dirFisica = UINT32_MAX;
    dirFisica = mmu(direccionLogica, tamRegistro);

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
    contextoEjecucion->rafagaCPUEjecutada = temporal_gettime(temporizador);  
}

void modificarMotivoDesalojo (t_comando comando, int numParametros, char * parm1, char * parm2, char * parm3, char * parm4, char * parm5) {
    char * (parametros[5]) = { parm1, parm2, parm3, parm4, parm5};
    contextoEjecucion->motivoDesalojo->motivo = comando;
    for (int i = 0; i < numParametros; i++)
        contextoEjecucion->motivoDesalojo->parametros[i] = string_duplicate(parametros[i]);
    contextoEjecucion->motivoDesalojo->parametrosLength = numParametros;
}

void liberarMemoria() {
    for (int i = 0; i <= cantParametros; i++) free(elementosInstruccion[i]);
    free(elementosInstruccion);
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

    switch(cantParametros) {
        case 0:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> ", contextoEjecucion->pid, listaComandos[instruccionActual]);
            break;
        case 1:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> -  <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1]);
            break;
        case 2:   
            log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2]);
            break;
        case 3:
            log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>, <%s>, <%s>", contextoEjecucion->pid, listaComandos[instruccionActual], elementosInstruccion[1], elementosInstruccion[2], elementosInstruccion[3]);
            break; 
    }

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

//MMU
uint32_t mmu(char* direccionLogica, int tamValor){
    //TODO: Implementacion para paginacion
    return UINT32_MAX; 
}
