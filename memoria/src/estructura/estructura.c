#include <estructura/estructura.h>
#include <commons/log.h>

//extern t_log* logger;
//extern t_log* loggerError;
int tamano_memoria_total = 0;
int memoria_usada = 0;

// Implementación de la memoria física
MemoriaFisica *inicializar_memoria_fisica(int tamano_pagina) {
    MemoriaFisica *mf = malloc(sizeof(MemoriaFisica));
    mf->memoria = malloc(NUM_MARCOS * 2048);
    for (int i = 0; i < NUM_MARCOS; i++) {
        mf->marcos[i].libre = true;
        mf->marcos[i].numero_pagina = -1;
        mf->marcos[i].pid = -1;
    }
    return mf;
}

void liberar_memoria_fisica(MemoriaFisica *mf) {
    free(mf->memoria);
    free(mf);
}

// Implementación de la tabla de páginas
TablaPaginas *inicializar_tabla_paginas() {
    TablaPaginas *tp = malloc(sizeof(TablaPaginas));
    for (int i = 0; i < CANT_PAGINAS; i++) {
        tp->entradas[i].valido = 0;
        tp->entradas[i].numero_marco = -1;
    }
    tp->paginas_asignadas = 0;  // Inicializa el contador de páginas asignadas
    return tp;
}

void liberar_tabla_paginas(TablaPaginas *tp) {
    if (tp != NULL) {
        // No es necesario liberar cada entrada individualmente ya que es un array estático
        // Solo liberamos la estructura TablaPaginas
        free(tp);
        log_info(logger, "Tabla de páginas liberada");
    } else {
        log_warning(logger, "Tabla de páginas es NULL");
    }
}

// Implementación del proceso
Proceso *inicializar_proceso(int pid, const char *archivo_pseudocodigo) {
    Proceso *proceso = malloc(sizeof(Proceso));
    proceso->pid = pid;
    //printf("tamaño del proceso %lu\n", sizeof(proceso->tabla_paginas));
    proceso->tabla_paginas = inicializar_tabla_paginas();
    proceso->pid=pid;
    // Leer archivo de pseudocódigo
    FILE *archivo = fopen(archivo_pseudocodigo, "r");
    if (!archivo) {
        perror("Error al abrir el archivo de pseudocódigo");
        free(proceso);
        return NULL;
    }
    
    proceso->numero_instrucciones = 0;
    proceso->instrucciones = NULL;
    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        proceso->numero_instrucciones++;
        proceso->instrucciones = realloc(proceso->instrucciones, proceso->numero_instrucciones * sizeof(char *));
        proceso->instrucciones[proceso->numero_instrucciones - 1] = string_duplicate(linea);
    }
    fclose(archivo);
    //printf("tamaño del proceso %lu\n", sizeof(proceso->tabla_paginas));
    return proceso;
}

void liberar_proceso(Proceso *proceso) {
    liberar_tabla_paginas(proceso->tabla_paginas);
    for (int i = 0; i < proceso->numero_instrucciones; i++) {
        free(proceso->instrucciones[i]);
    }
    free(proceso->instrucciones);
    free(proceso);
}

char *obtener_instruccion(Proceso *proceso, int program_counter) {
    if (program_counter < 0 || program_counter >= proceso->numero_instrucciones) {
        return NULL;
    }
    return proceso->instrucciones[program_counter];
}

// Traduce una dirección lógica a una dirección física
// Ya se encarga la mmu
/*void *traducir_direccion(MemoriaFisica *mf, Proceso *proceso, void *direccion_logica) {
    int tam_pagina = confGetInt("TAM_PAGINA");
    unsigned long dir = (unsigned long)direccion_logica;
    int numero_pagina = dir / tam_pagina;
    int desplazamiento = dir % tam_pagina;

    if (numero_pagina < 0 || numero_pagina >= NUM_PAGINAS || !proceso->tabla_paginas->entradas[numero_pagina].valido) {
        return NULL; // Dirección no válida
    }

    int numero_marco = proceso->tabla_paginas->entradas[numero_pagina].numero_marco;
    //return (char*)mf->memoria + numero_marco * tam_pagina + desplazamiento;
    return mf->memoria + numero_marco * tam_pagina + desplazamiento;
}*/

bool hay_memoria_disponible(int tamano_requerido) {
    return (memoria_usada + tamano_requerido <= tamano_memoria_total);
}

bool hay_espacio_en_tabla_paginas(Proceso *proceso) {
    return (proceso->tabla_paginas->paginas_asignadas < CANT_PAGINAS);
}

void *leer_memoria(MemoriaFisica *mf, Proceso *proceso, int direccion_fisica, size_t size) {
    if (direccion_fisica < 0 || direccion_fisica + size > tamano_memoria_total) {
        log_error(loggerError, "Error: Dirección física fuera de los límites de la memoria.");
        return NULL;
    }

    void *buffer = malloc(size);
    if (!buffer) {
        log_error(loggerError, "Error: No se pudo asignar memoria para el buffer de lectura.");
        return NULL;
    }

    memcpy(buffer, (char *)mf->memoria + direccion_fisica, size);

    // Log del acceso de lectura
    log_info(logger, "PID: %d - Acción: LEER - Dirección física: %d - Tamaño: %zu",
             proceso->pid, direccion_fisica, size);

    return buffer;
}

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
}

bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina) {
    int tam_pagina = confGetInt("TAM_PAGINA");

    if (!hay_memoria_disponible(tam_pagina)) {
        log_error(loggerError, "Error: No hay suficiente memoria para asignar una nueva página.");
        return false;
    }

    if (!hay_espacio_en_tabla_paginas(proceso)) {
        log_error(loggerError, "Error: No hay suficiente espacio en la tabla de páginas para asignar una nueva página.");
        return false;
    }

    for (int i = 0; i < NUM_MARCOS; i++) {
        if (mf->marcos[i].libre) {
            mf->marcos[i].libre = 0;
            mf->marcos[i].numero_pagina = numero_pagina;
            mf->marcos[i].pid = proceso->pid;
            mf->marcos[i].proceso = proceso;

            proceso->tabla_paginas->entradas[numero_pagina].valido = 1;
            proceso->tabla_paginas->entradas[numero_pagina].numero_marco = i;
            proceso->tabla_paginas->paginas_asignadas++;

            memoria_usada += tam_pagina;

            return true;
        }
    }

    log_error(loggerError, "Error: No hay marcos libres para asignar.");
    return false;
}