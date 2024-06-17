#include <estructura/estructura.h>
#include <commons/log.h>

//extern t_log* logger;
//extern t_log* loggerError;

// Implementación de la memoria física
MemoriaFisica *inicializar_memoria_fisica() {
    MemoriaFisica *mf = malloc(sizeof(MemoriaFisica));
    mf->memoria = malloc(NUM_MARCOS * TAMANO_PAGINA);
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
    for (int i = 0; i < NUM_PAGINAS; i++) {
        tp->entradas[i].valido = 0;
        tp->entradas[i].numero_marco = -1;
    }
    tp->paginas_asignadas = 0;  // Inicializa el contador de páginas asignadas
    return tp;
}

void liberar_tabla_paginas(TablaPaginas *tp) {
    free(tp);
}

// Implementación del proceso
Proceso *inicializar_proceso(int pid, const char *archivo_pseudocodigo) {
    Proceso *proceso = malloc(sizeof(Proceso));
    proceso->pid = pid;
    //printf("tamaño del proceso %lu\n", sizeof(proceso->tabla_paginas));
    proceso->tabla_paginas = inicializar_tabla_paginas();
    
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

// Asigna una página en la memoria física
/*bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina) {
    if (numero_pagina < 0 || numero_pagina >= NUM_PAGINAS) {
        return false;
    }
    // Encuentra un marco libre
    int numero_marco = -1;
    for (int i = 0; i < NUM_MARCOS; i++) {
        if (mf->marcos[i].libre) {
            numero_marco = i;
            break;
        }
    }
    if (numero_marco == -1) {
        return false; // No hay marcos libres
    }
    mf->marcos[numero_marco].libre = false;
    mf->marcos[numero_marco].numero_pagina = numero_pagina;
    mf->marcos[numero_marco].pid = proceso->pid;
    proceso->tabla_paginas->entradas[numero_pagina].valido = 1;
    proceso->tabla_paginas->entradas[numero_pagina].numero_marco = numero_marco;
    proceso->tabla_paginas->paginas_asignadas++;  // Incrementa el contador de páginas asignadas

    return true;
}*/

// Traduce una dirección lógica a una dirección física
void *traducir_direccion(MemoriaFisica *mf, Proceso *proceso, void *direccion_logica) {
    unsigned long dir = (unsigned long)direccion_logica;
    int numero_pagina = dir / TAMANO_PAGINA;
    int desplazamiento = dir % TAMANO_PAGINA;

    if (numero_pagina < 0 || numero_pagina >= NUM_PAGINAS || !proceso->tabla_paginas->entradas[numero_pagina].valido) {
        return NULL; // Dirección no válida
    }

    int numero_marco = proceso->tabla_paginas->entradas[numero_pagina].numero_marco;
    return mf->memoria + numero_marco * TAMANO_PAGINA + desplazamiento;
}



bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina) {
    if (numero_pagina < 0 || numero_pagina >= NUM_PAGINAS) {
        return false; // Número de página fuera de rango
    }
    // Busca un marco libre disponible para asignar la página
    for (int i = 0; i < NUM_MARCOS; i++) {
        if (mf->marcos[i].libre) {
            // Se encontró un marco libre, asigna la página
            mf->marcos[i].libre = false;
            mf->marcos[i].numero_pagina = numero_pagina;
            mf->marcos[i].pid = proceso->pid;
            proceso->tabla_paginas->entradas[numero_pagina].valido = 1;
            proceso->tabla_paginas->entradas[numero_pagina].numero_marco = i;
            proceso->tabla_paginas->paginas_asignadas++; // Incrementa el contador de páginas asignadas
            return true;
        }
    }
    // Si no se encontró ningún marco libre
    return false;
}