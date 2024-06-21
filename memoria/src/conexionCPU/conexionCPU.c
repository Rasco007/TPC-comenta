#include <conexionCPU/conexionCPU.h>

int tiempo;
//MemoriaFisica *mf;
//t_log *logger;
//t_config *config;
char* valorLeido;
int indice;
char* instruccion; 

void limpiarBuffer(int socketCliente){
    int size;
    void* buffer = recibirBuffer(socketCliente, &size);
    free(buffer);
}

// Recibo peticiones de CPU y mando respuesta
int ejecutarServidorCPU(int *socketCliente) {
    tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");
    while (1) {
        int peticion = recibirOperacion(*socketCliente);
        log_info(logger, "Se recibió petición %d del CPU", peticion);

        switch (peticion) {

            case READ:
            log_info(logger, "Llegue al case READ");
                recibirPeticionDeLectura(*socketCliente);
                enviarValorObtenido(*socketCliente);
                break;
            case WRITE:
                recibirPeticionDeEscritura(*socketCliente);
                enviarMensaje("OK", *socketCliente);
                break;
            case MMU:// recibirPeticionDeLectura(*socketCliente);
                log_info(logger, "Llegue al case MMU, numero de case: %d", peticion);
                int pid,pag;
                char marco[sizeof(int)];
                recibirEnteros(*socketCliente,&pid,&pag);
                BuscarYEnviarMarco(pid, pag, marco,*socketCliente);
                limpiarBuffer(*socketCliente);
                break;
            case -1:
                log_error(logger, "El CPU se desconectó");
                return EXIT_FAILURE;
                break;
            case PAQUETE:
                //Se usaria el indice para buscar en la lista donde almacenemos las instrucciones
                log_info(logger, "Se recibió la peticion de CPU");
                indice=recibirPaquete(*socketCliente);
                //INSERTAR SEMAFORO O ALGO...
                //Espero a que memoria almacene en instruccion la linea que se solicita y la mando a cpu
                enviarMensaje(instruccion,socketCliente);
                break;
            default:
                log_warning(logger, "Operación desconocida del CPU.");
                break;
        }
    }
    return EXIT_SUCCESS;
}

void recibirEnteros(int socket, int *pid, int *pagina) {
    char buffer[2048];
    // Recibir el mensaje del servidor
    int bytes_recibidos = recv(socket, buffer, sizeof(buffer), 0);
    
    if (bytes_recibidos < 0) {
        perror("Error al recibir el mensaje");
        return;
    }
    memcpy(pid ,buffer+sizeof(op_code), sizeof(int));
    memcpy(pagina, buffer+sizeof(int)+sizeof(op_code), sizeof(int));
    printf("Recibido PID: %d, Página: %d\n", *pid, *pagina);
}

void BuscarYEnviarMarco (int pid, int pagina,char* marco,int socketCliente){
    int frame=0;
    for (int i=0; i<NUM_MARCOS; i++){
        if (mf->marcos[i].pid == pid && mf->marcos[i].numero_pagina == pagina){
            log_info(logger, "Marco encontrado: %d", i);
            frame= i;
            break;
        }
    }
    sprintf(marco, "%d", frame);
    log_info(logger, "PID: <%d> - Pagina: <%d> - Marco: <%s>", pid, pagina, marco);
    enviarMensaje(marco, socketCliente);
}

char* leer(int32_t direccionFisica, int tamanio) {
    usleep(tiempo * 1000); // Simula el retardo

    char* punteroDireccionFisica = (char*)mf->memoria + direccionFisica;
    char* valor = malloc(sizeof(char) * tamanio);
    
    memcpy(valor, punteroDireccionFisica, tamanio);

    return valor;
}

void recibirPeticionDeLectura(int socketCPU) {
    int size, desplazamiento = 0, pid, tamanio;
    int32_t direccionFisica;

    void* buffer = recibirBuffer(socketCPU, &size);
    desplazamiento += sizeof(uint32_t);
    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(uint32_t) + sizeof(int);
    memcpy(&direccionFisica, buffer + desplazamiento, sizeof(int32_t));
    desplazamiento += sizeof(uint32_t) + sizeof(int);
    memcpy(&tamanio, buffer + desplazamiento, sizeof(int));

    valorLeido = leer(direccionFisica, tamanio);
    valorLeido = realloc(valorLeido, tamanio + 1);
    valorLeido[tamanio] = '\0';

    log_info(logger, "PID: <%d> - Acción: <%s> - Dirección física: <%d> - Tamaño: <%d> - Origen: <%s>", pid, "LEER", direccionFisica, tamanio, "CPU");

    free(buffer);
}

void recibirPeticionDeEscritura(int socketCPU) {
    int size, desplazamiento = 0, tamanio, pid;
    int32_t direccionFisica;

    void* buffer = recibirBuffer(socketCPU, &size);
    desplazamiento += sizeof(uint32_t);
    memcpy(&pid, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(uint32_t) + sizeof(int);
    memcpy(&direccionFisica, buffer + desplazamiento, sizeof(int32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    char* valorAEscribir = malloc(sizeof(char) * tamanio);
    memcpy(valorAEscribir, buffer + desplazamiento, sizeof(char) * tamanio);
    
    tamanio--;

    escribir(valorAEscribir, direccionFisica, tamanio);

    log_info(logger, "PID: <%d> - Acción: <%s> - Dirección física: <%d> - Tamaño: <%d> - Origen: <%s>", pid, "ESCRIBIR", direccionFisica, tamanio, "CPU");
    free(buffer);
}

void enviarValorObtenido(int socketCPU) {
    enviarMensaje(valorLeido, socketCPU);
    free(valorLeido);
    log_info(logger, "Envie valor a CPU");
}

void escribir(char* valor, int32_t direccionFisica, int tamanio) {
    usleep(tiempo * 1000); // Simula el retardo

    char* punteroADirFisica = (char*)mf->memoria + direccionFisica;
    
    memcpy(punteroADirFisica, valor, tamanio);

    free(valor);
}

Proceso *ajustar_tamano_proceso(MemoriaFisica *mf, Proceso *proceso, int nuevo_tamano) {
    int tam_pagina = confGetInt("TAM_PAGINA");
    // Calcula el número total de páginas necesarias para el nuevo tamaño
    int paginas_necesarias = nuevo_tamano / tam_pagina;
    if (nuevo_tamano % tam_pagina != 0)
        paginas_necesarias++;
    // Si el número de páginas es el mismo, no hay cambio necesario
    if (paginas_necesarias == proceso->tabla_paginas->paginas_asignadas)
        return proceso;
    if (paginas_necesarias > NUM_MARCOS){
        enviarMensaje("No hay suficiente espacio en memoria para asignar más páginas", sockets[0]);
        log_error(loggerError, "Out of Memory al intentar asignar %d bytes al proceso PID: %d",nuevo_tamano,proceso->pid);
        return proceso;
    }
    // Si se requieren más páginas, asignar las nuevas páginas
    if (paginas_necesarias > proceso->tabla_paginas->paginas_asignadas) {
        // Registro de ampliación del proceso
        //int tamano_a_ampliar = (paginas_necesarias - proceso->tabla_paginas->paginas_asignadas) * tam_pagina;
        int nuevoTamanio = paginas_necesarias * tam_pagina;
        log_info(logger, "Ampliación de Proceso: PID: %d - Tamaño Actual: %d bytes - Tamaño a Ampliar: %d bytes",proceso->pid, proceso->tabla_paginas->paginas_asignadas * tam_pagina, nuevoTamanio);
        for (int i = proceso->tabla_paginas->paginas_asignadas; i < paginas_necesarias; i++) {
            if (!asignar_pagina(mf, proceso, i)) {
                log_error(loggerError, "Error al asignar página %d al proceso", i);
                return NULL; // Abortar si no se puede asignar una página
            }
        }
    } else { // Si se requieren menos páginas, liberar las páginas extras
        // Registro de reducción del proceso
        //int tamano_a_reducir = (proceso->tabla_paginas->paginas_asignadas - paginas_necesarias) * tam_pagina;
        int nuevoTamanio = paginas_necesarias * tam_pagina;
        log_info(logger,"Reducción de Proceso: PID: %d - Tamaño Actual: %d bytes - Tamaño a Reducir: %d bytes",proceso->pid, proceso->tabla_paginas->paginas_asignadas * tam_pagina, nuevoTamanio);
        for (int i = proceso->tabla_paginas->paginas_asignadas - 1; i >= paginas_necesarias; i--) {
            // Liberar la página i
            proceso->tabla_paginas->entradas[i].valido = 0;
            proceso->tabla_paginas->entradas[i].numero_marco = -1;
            mf->marcos[proceso->tabla_paginas->entradas[i].numero_marco].libre = true;
            mf->marcos[proceso->tabla_paginas->entradas[i].numero_marco].proceso = NULL;
        }
    }
    // Actualiza el número de páginas asignadas en la tabla de páginas del proceso
    proceso->tabla_paginas->paginas_asignadas = paginas_necesarias;
    return proceso;
}

/*Proceso *ajustar_tamano_proceso(MemoriaFisica *mf, Proceso *proceso, int nuevo_tamano) {
    // Calcula el número total de páginas necesarias para el nuevo tamaño
    int paginas_necesarias = nuevo_tamano / tam_pagina;
    if (nuevo_tamano % tam_pagina != 0) {
        paginas_necesarias++;
    }

    // Si el número de páginas es el mismo, no hay cambio necesario
    if (paginas_necesarias == proceso->tabla_paginas->paginas_asignadas) {
        return proceso;
    }

    // Si se requieren más páginas, asignar las nuevas páginas
    if (paginas_necesarias > proceso->tabla_paginas->paginas_asignadas) {
        for (int i = proceso->tabla_paginas->paginas_asignadas; i < paginas_necesarias; i++) {
            if (!asignar_paginaposta(mf, proceso, i)) {
                log_error(loggerError, "Error al asignar página %d al proceso", i);
                return NULL; // Abortar si no se puede asignar una página
            }
        }
    } else { // Si se requieren menos páginas, liberar las páginas extras
        for (int i = proceso->tabla_paginas->paginas_asignadas - 1; i >= paginas_necesarias; i--) {
            // Liberar la página i
            proceso->tabla_paginas->entradas[i].valido = 0;
            proceso->tabla_paginas->entradas[i].numero_marco = -1;
            mf->marcos[proceso->tabla_paginas->entradas[i].numero_marco].libre = true;
            mf->marcos[proceso->tabla_paginas->entradas[i].numero_marco].proceso = NULL;
        }
    }

    // Actualiza el número de páginas asignadas en la tabla de páginas del proceso
    proceso->tabla_paginas->paginas_asignadas = paginas_necesarias;

    return proceso;
}*/
