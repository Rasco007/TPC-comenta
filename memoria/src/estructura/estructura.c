#include <estructura/estructura.h>
#include <commons/log.h>

// Implementación de la memoria física
MemoriaFisica *inicializar_memoria_fisica(int tamano_pagina) {
    MemoriaFisica *mf = malloc(sizeof(MemoriaFisica));
    mf->memoria=malloc(TAM_MEMORIA);
    mf->tablaMarcosLibres = bitarray_create(mf->memoria, TAM_MEMORIA);
}

void liberar_memoria_fisica(MemoriaFisica *mf) {
    free(mf->memoria);
    free(mf);
}

// Implementación de la tabla de páginas

TablaPaginas *inicializar_tabla_paginas() {
    TablaPaginas *tp = malloc(sizeof(TablaPaginas));
    tp->entradas=list_create();
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
    proceso->tabla_paginas = NULL;
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


/* VER DESPUES CON SANTY, CAMBIAR NUM MARCOS POR SIZE DEL BITARRAY
bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina) {
    if (numero_pagina < 0 || numero_pagina >= CANT_PAGINAS) {
        return false; // Número de página fuera de rango
    }
    // Busca un marco libre disponible para asignar la página
    for (int i = 0; i < NUM_MARCOS; i++) {
        if (mf->marcos[i].libre) {
            // Se encontró un marco libre, asigna la página
            mf->marcos[i].libre = false;
            mf->marcos[i].numero_pagina = numero_pagina;
            mf->marcos[i].pid = proceso->pid;
            mf->marcos[i].proceso = proceso; //va?
            proceso->tabla_paginas->entradas[numero_pagina].valido = 1;
            proceso->tabla_paginas->entradas[numero_pagina].numero_marco = i;
            proceso->tabla_paginas->paginas_asignadas++; // Incrementa el contador de páginas asignadas
            return true;
        }
    }
    // Si no se encontró ningún marco libre
    return false;
}*/