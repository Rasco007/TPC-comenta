#include <conexionCPU/conexionCPU.h>

int tiempo;
char* valorLeido;
int indice;
int pid;
char* instruccion; 

void limpiarBuffer(int socketCliente){
    int size;
    void* buffer = recibirBuffer(socketCliente, &size);
    free(buffer);
}

void recibirEnteros2(int socket, int *pid, int *indice) {
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

// Recibo peticiones de CPU y mando respuesta
int ejecutarServidorCPU(int *socketCliente) {
   // logger = cambiarNombre(logger, "conexion con CPU - Memoria");
    tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");
    while (1) {
        int peticion = recibirOperacion(*socketCliente);
        //log_info(logger, "Se recibió petición %d del CPU", peticion);

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
            case MMU:
                log_info(logger, "Llegue al case MMU, numero de case: %d", peticion);
                int pid,pag;
                char marco[sizeof(int)];
                recibirEnteros(*socketCliente,&pid,&pag);
                BuscarYEnviarMarco(pid, pag, marco,*socketCliente);
                //limpiarBuffer(*socketCliente);
                break;
            case -1:
                log_error(logger, "El CPU se desconectó");
                return EXIT_FAILURE;
                break;
            case PAQUETE:
                //Se usaria el indice para buscar en la lista donde almacenemos las instrucciones
                //log_info(logger, "Se recibió la peticion de CPU"); 
                recibirEnteros2(*socketCliente, &pid, &indice);
                //log_info(logger, "Indice: %d - PID: %d",indice,pid);
                Proceso *proceso = buscar_proceso_por_pid(pid); //Busco el proceso correspondiente
                //log_info(logger, "Proc: %d",proceso->pid);
                instruccion = obtener_instruccion(proceso,indice); //Obtengo la instruccion correspondiente
                //log_info(logger, "Instruccion: %s",instruccion);
                sleep(config_get_int_value(config, "RETARDO_RESPUESTA")/1000); //Agrego retardo
                enviarMensaje(instruccion,*socketCliente); 
                break;
            case RESIZE: //VER
                //log_info(logger, "Se recibió la petición de CPU para redimensionar un proceso");
                int nuevo_tamano;
                recibirEnteros2(*socketCliente, &pid, &nuevo_tamano);
                log_info(logger, "PID: %d - Nuevo tamaño: %d", pid, nuevo_tamano);
                proceso = buscar_proceso_por_pid(pid);
                if (proceso == NULL) {
                    log_error(loggerError, "No se encontró el proceso con PID: %d", pid);
                    break;
                }
                proceso = ajustar_tamano_proceso(mf, proceso, nuevo_tamano);
                break;
            case 104: // PARA LEER POR COPY STRING
                usleep(1000*1000);
                log_info(logger, "MEMORIA envía mensaje a CPU segun direccion y tamaño");
                int dir2, tamano, pid2;
                recibirDireccionyTamano(*socketCliente, &dir2, &pid2, &tamano);
                printf("Tamaño: %d\n", tamano);
                char* datosLeidos = malloc(tamano);
                memcpy(datosLeidos, (char*)mf->memoria + dir2, tamano);
                datosLeidos[tamano] = '\0';
                log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion Física: <%d> - Valor: <%s>", pid2, dir2, datosLeidos);
                send(*socketCliente, datosLeidos, tamano, 0);
                free(datosLeidos);
                break;
            case 105: //PARA ESCRIBIR POR COPY STRING
                usleep(1000*1000);
                log_info(logger, "CPU envía mensaje a escribir en memoria");
                int dir, pid3;
                char cadena[2048]="";
                recibirDirYCadena(*socketCliente, &dir, &pid3, cadena);
                cadena[strlen(cadena)] = '\0';
                log_info(logger, "PID: <%d> - Accion: <ESCRIBIR> - Direccion Física: <%d> - Valor: <%s>", pid3, dir, cadena); 
                memcpy((char*)mf->memoria + dir, cadena, strlen(cadena));
                char* datoEscrito= malloc(strlen(cadena));//ACA VERIFICO QUE SE ESCRIBIO BIEN EN MEMORIA!!!!!!!!
                memcpy(datoEscrito, (char*)mf->memoria + dir, strlen(cadena));
                datoEscrito[strlen(cadena)] = '\0';
                printf("Dato escrito: %s\n", datoEscrito);
                free(datoEscrito);
                char *mensje="ok";
                send(*socketCliente, &mensje, sizeof(mensje), 0);
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
    for (int i=0; i<list_size(mf->listaProcesos); i++){
        Proceso *proceso = list_get(mf->listaProcesos,i);

    for (int i = 0; i < list_size(proceso->tabla_paginas->entradas); i++) {
    EntradaTablaPaginas* entrada = list_get(proceso->tabla_paginas->entradas, i);
        if (proceso->pid == pid && entrada->numero_pagina == pagina) {
             log_info(logger, "Marco encontrado: %d", i);
            frame = entrada->numero_marco;
        }
    }

    }
    sprintf(marco, "%d", frame);
    log_info(logger, "PID: <%d> - Pagina: <%d> - Marco: <%s>", pid, pagina, marco);
    enviarMensaje(marco, socketCliente);
}

char* leer(uint32_t pid, uint32_t direccionFisica, uint32_t tamanio) {
    if (direccionFisica < 0 || direccionFisica + tamanio > TAM_MEMORIA) {
        log_error(loggerError, "Error: Dirección física fuera de los límites de la memoria.");
        return NULL;
    }
    Proceso* proceso=buscar_proceso_por_pid(pid);
    

    int indicePaginaInicial = direccionFisica / TAM_PAGINA;
    int indicePaginaFinal = (direccionFisica + tamanio - 1) / TAM_PAGINA;
    
    //Con esto obtendria los marcos que tengo que leer
    t_list* listaMarcosInvolucrados=list_create();
    for(int i=indicePaginaInicial; i<=indicePaginaFinal; i++){ //Como son contiguos...
        EntradaTablaPaginas* entrada = list_get(proceso->tabla_paginas->entradas, i);
        int marco = entrada->numero_marco;
        list_add(listaMarcosInvolucrados, marco);
    }
    //Falta la lectura
 
}

/*FUNCION TOMY
void *leer_memoria(Proceso *proceso, int direccion_fisica, size_t size) {
    if (direccion_fisica < 0 || direccion_fisica + size > TAM_MEMORIA) {
        log_error(loggerError, "Error: Dirección física fuera de los límites de la memoria.");
        return NULL;
    }

    void *buffer = malloc(size);
    if (!buffer) {
        log_error(loggerError, "Error: No se pudo asignar memoria para el buffer de lectura.");
        return NULL;
    }
    //VER CUANDO SE TERRMINAN LAS   PAAGINAAAAAASSSS
    memcpy(buffer, (char *)mf->memoria + direccion_fisica, size);

    // Log del acceso de lectura
    log_info(logger, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %zu",
             proceso->pid, direccion_fisica, size);

    return buffer;
}*/

void recibirPeticionDeLectura(int socketCPU) {
    uint32_t size, desplazamiento = 0, pid, tamanio;
    int32_t direccionFisica;

    void* buffer = recibirBuffer(socketCPU, &size);
    desplazamiento += sizeof(uint32_t);
    memcpy(&pid, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t) + sizeof(uint32_t);
    memcpy(&direccionFisica, buffer + desplazamiento, sizeof(int32_t));
    desplazamiento += sizeof(uint32_t) + sizeof(uint32_t);
    memcpy(&tamanio, buffer + desplazamiento, sizeof(uint32_t));

    valorLeido = leer(pid,direccionFisica, tamanio);
    valorLeido = realloc(valorLeido, tamanio + 1);
    valorLeido[tamanio] = '\0';

    log_info(logger, "PID: <%d> - Acción: <%s> - Dirección física: <%d> - Valor:<%s>", pid, "LEER", direccionFisica, valorLeido);
    free(buffer);
}

void recibirPeticionDeEscritura(int socketCPU) {
    uint32_t size, desplazamiento = 0, tamanio, pid;
    int32_t direccionFisica;

    void* buffer = recibirBuffer(socketCPU, &size);
    desplazamiento += sizeof(uint32_t);
    memcpy(&pid, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t) + sizeof(uint32_t);
    memcpy(&direccionFisica, buffer + desplazamiento, sizeof(int32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&tamanio, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    char* valorAEscribir = malloc(sizeof(char) * tamanio);
    memcpy(valorAEscribir, buffer + desplazamiento, sizeof(char) * tamanio);

    escribir(valorAEscribir, direccionFisica, tamanio);

    log_info(logger, "PID: <%d> - Acción: <%s> - Dirección física: <%d> - Valor: <%s>", pid, "ESCRIBIR", direccionFisica, valorAEscribir);
    free(buffer);
}

void enviarValorObtenido(int socketCPU) {
    enviarMensaje(valorLeido, socketCPU);
    free(valorLeido);
    log_info(logger, "Envie valor a CPU");
}

void escribir(char* valor, uint32_t direccionFisica, uint32_t tamanio) {
    if (direccionFisica < 0 || direccionFisica + tamanio > TAM_MEMORIA) {
        log_error(loggerError, "Error: Dirección física fuera de los límites de la memoria.");
        return;
    }

    char* punteroADirFisica = (char*)mf->memoria + direccionFisica; //Apunto al comienzo del espacio de usuario
    
    memcpy(punteroADirFisica, valor, tamanio); //Escribo el valor. Inicio=DF y el tamanio es el desplazamiento

    free(valor);
}

/* FUNCION TOMY
void escribir_memoria(MemoriaFisica *mf, Proceso *proceso, int direccion_fisica, const void *data, size_t size) {
    if (direccion_fisica < 0 || direccion_fisica + size > tamano_memoria_total) {
        log_error(loggerError, "Error: Dirección física fuera de los límites de la memoria.");
        return;
    }

    memcpy((char *)mf->memoria + direccion_fisica, data, size);

    // Log del acceso de escritura
    log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %zu",
             proceso->pid, direccion_fisica, size);
    
    log_info(logger, "Escritura en memoria EXITOSA");
}*/

Proceso *ajustar_tamano_proceso(MemoriaFisica *mf, Proceso *proceso, int nuevo_tamano) {
    int tam_pagina = confGetInt("TAM_PAGINA");
    // Calcula el número total de páginas necesarias para el nuevo tamaño
    int paginas_necesarias = nuevo_tamano / tam_pagina;
    if (nuevo_tamano % tam_pagina != 0)
        paginas_necesarias++;
    // Si el número de páginas es el mismo, no hay cambio necesario
    if (paginas_necesarias == proceso->tabla_paginas->paginas_asignadas)
        return proceso;
    if (paginas_necesarias > CANT_FRAMES){
        enviarMensaje("No hay suficiente espacio en memoria para asignar más páginas", sockets[0]);
        log_error(loggerError, "Out of Memory al intentar asignar %d bytes al proceso PID: %d",nuevo_tamano,proceso->pid);
        return proceso;
    }
    //log_warning(logger, "Cantidad de páginas necesarias: %d", paginas_necesarias);
    // Si se requieren más páginas, asignar las nuevas páginas
    if (paginas_necesarias > proceso->tabla_paginas->paginas_asignadas) {
        // Registro de ampliación del proceso
        int tamano_a_ampliar = (paginas_necesarias - proceso->tabla_paginas->paginas_asignadas) * tam_pagina;
        //int nuevoTamanio = paginas_necesarias * tam_pagina;
        log_info(logger, "Ampliación de Proceso: PID: %d - Tamaño Actual: %d bytes - Tamaño a Ampliar: %d bytes",proceso->pid, proceso->tabla_paginas->paginas_asignadas * tam_pagina, tamano_a_ampliar);
        for (int i = proceso->tabla_paginas->paginas_asignadas; i < paginas_necesarias; i++) {
            if (!asignar_pagina(mf, proceso, i)) {
                log_error(loggerError, "Error al asignar página %d al proceso", i);
                return NULL; // Abortar si no se puede asignar una página
            }
        }
    } else { // Si se requieren menos páginas, liberar las páginas extras
        // Registro de reducción del proceso
        int tamano_a_reducir = (proceso->tabla_paginas->paginas_asignadas - paginas_necesarias) * tam_pagina;
        //int nuevoTamanio = paginas_necesarias * tam_pagina;
        log_info(logger,"Reducción de Proceso: PID: %d - Tamaño Actual: %d bytes - Tamaño a Reducir: %d bytes",proceso->pid, proceso->tabla_paginas->paginas_asignadas * tam_pagina, tamano_a_reducir);
       
            
        for (int i = proceso->tabla_paginas->paginas_asignadas - 1; i >= paginas_necesarias; i--) {
             EntradaTablaPaginas *entrada = malloc(sizeof(EntradaTablaPaginas));
             entrada=list_get(proceso->tabla_paginas->entradas, i);
             int marco = entrada->numero_marco;

            //TODDDDO: PONERRRR   EN LISTA DE  MARCOS LOS QUUE AHHHORA ESTAAARIIIAN DISPONIBLESS

            // Liberar la página i
            list_remove(proceso->tabla_paginas->entradas,i);
            list_replace(mf->listaMarcosLibres, marco,false); //creo que se pone - 1 porque es el indice dee una lista y no existe el marco 0
            //reemmplazaaaa vaalor marcando false como  disponible
        }
    }
    // Actualiza el número de páginas asignadas en la tabla de páginas del proceso
    proceso->tabla_paginas->paginas_asignadas = paginas_necesarias;
    log_info(logger,"cant. de paginas asignadas: %d",proceso->tabla_paginas->paginas_asignadas);
    //contar cantidad de marcos libres
    int marcosLibres=0;
    for (int i = 0; i < list_size(mf->listaMarcosLibres); i++) {
        if (list_get(mf->listaMarcosLibres,i) == false)
            marcosLibres++;
    }
    log_info(logger,"Marcos libres: %d",marcosLibres);
    //indicar en que posicion de la lista de marcos libres se encuentran los marcos ocupados
    for (int i = 0; i < list_size(mf->listaMarcosLibres); i++) {
        if (list_get(mf->listaMarcosLibres,i) == true)
            log_info(logger,"Marco ocupado: %d",i);
    }
    return proceso;    
}

void *traducir_direccion(MemoriaFisica *mf, Proceso *proceso, void *direccion_logica) {
    int tam_pagina = confGetInt("TAM_PAGINA");
    unsigned long dir = (unsigned long)direccion_logica;
    int numero_pagina = dir / tam_pagina;
    int desplazamiento = dir % tam_pagina;
    EntradaTablaPaginas* entrada=list_get(proceso->tabla_paginas->entradas,numero_pagina);

    if (numero_pagina < 0 || numero_pagina >= CANT_PAGINAS || !entrada->valido) { //VER CAMBIOS DE ESTA LINEA
        return NULL; // Dirección no válida
    }
    
    int numero_marco=entrada->numero_marco;
    //int numero_marco = proceso->tabla_paginas->entradas[numero_pagina].numero_marco; ESTO ESTABA ANTES
    return mf->memoria + numero_marco * tam_pagina + desplazamiento;
}