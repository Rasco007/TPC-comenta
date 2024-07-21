#include <estructura/estructura.h>
#include <commons/log.h>

// Implementación de la memoria física
MemoriaFisica *inicializar_memoria_fisica() {
    MemoriaFisica *mf = malloc(sizeof(MemoriaFisica));
    mf->memoria = malloc(TAM_MEMORIA);
    mf->listaMarcosLibres = list_create();
    mf->listaProcesos = list_create();
    for(int i=0;i<TAM_MEMORIA/TAM_PAGINA;i++){
        list_add(mf->listaMarcosLibres,false);
    }
    return mf;
}

void liberar_memoria_fisica(MemoriaFisica *mf) {
    free(mf->memoria);
    list_destroy(mf->listaMarcosLibres);
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
    //logger=cambiarNombre(logger,"Liberacion tabla de paginas-Memoria");
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

    proceso->tabla_paginas = inicializar_tabla_paginas();
    log_info(logger, "Creacion de tabla de paginas para proceso PID: <%d> - Tamaño: <%d> Páginas", pid, CANT_PAGINAS);
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
    list_add(mf->listaProcesos,proceso);
    
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

//TODOOOOO: VER DESPUES CON SANTY, CAMBIAR NUM MARCOS POR SIZE DEL BITARRAY
bool asignar_pagina(MemoriaFisica *mf, Proceso *proceso, int numero_pagina) {
    if (numero_pagina < 0 || numero_pagina >= CANT_PAGINAS) {
        return false; // Número de página fuera de rango
    }
    // Busca un marco libre disponible para asignar la página
    for (int i = 0; i < list_size(mf->listaMarcosLibres); i++) {
        if (list_get(mf->listaMarcosLibres,i) ==  false) {
            // Se encontró un marco libre, asigna la página
            EntradaTablaPaginas *entrada = malloc(sizeof(EntradaTablaPaginas));
            entrada->numero_marco= i;
            entrada->valido =1;
            list_add(proceso->tabla_paginas->entradas, entrada);
            proceso->tabla_paginas->paginas_asignadas++; // Incrementa el contador de páginas asignadas
            list_replace(mf->listaMarcosLibres,i,true);
            return true;
        }
    }
    // Si no se encontró ningún marco libre
    return false;
}