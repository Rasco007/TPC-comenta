#include <pseudocodigo/pseudocodigo.h>
#include <commons/log.h>

extern t_log* logger;
extern t_log* loggerError;

// Función para leer pseudocódigo desde un archivo
Pseudocodigo* leerPseudocodigo(const char* ruta) {
    FILE* archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        log_error(loggerError, "Error al abrir el archivo de pseudocódigo: %s", ruta);
        return NULL;
    }

    Pseudocodigo* pseudo = malloc(sizeof(Pseudocodigo));
    pseudo->cantidad_instrucciones = 0;
    pseudo->instrucciones = NULL;

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        pseudo->cantidad_instrucciones++;
        pseudo->instrucciones = realloc(pseudo->instrucciones, pseudo->cantidad_instrucciones * sizeof(char*));
        pseudo->instrucciones[pseudo->cantidad_instrucciones - 1] = strdup(linea);
    }

    fclose(archivo);
    return pseudo;
}

// Función para liberar la memoria del pseudocódigo
void liberarPseudocodigo(Pseudocodigo* pseudo) {
    for (int i = 0; i < pseudo->cantidad_instrucciones; i++) {
        free(pseudo->instrucciones[i]);
    }
    free(pseudo->instrucciones);
    free(pseudo);
}

// Función para obtener una instrucción del pseudocódigo
char* obtenerInstruccion(Pseudocodigo* pseudo, int indice) {
    if (indice >= 0 && indice < pseudo->cantidad_instrucciones) {
        return pseudo->instrucciones[indice];
    } else {
        return NULL;
    }
}

// Función para crear un proceso
void crear_proceso(Memoria* memoria, int pid, int tamanio, const char* ruta_pseudocodigo) {
    Pseudocodigo* pseudo = leerPseudocodigo(ruta_pseudocodigo);
    if (pseudo == NULL) {
        log_error(loggerError, "Error al leer el archivo de pseudocódigo para el proceso %d", pid);
        return;
    }

    // Simulando la asignación de marcos a páginas para el proceso.
    for (int i = 0; i < tamanio; i++) {
        memoria->tabla_paginas->entradas[i].numeroDeMarco = i; // Asigna el marco
        memoria->tabla_paginas->entradas[i].bitDeValidez = 1; // Marca la página como válida
    }

    log_info(logger, "Proceso %d creado con %d páginas.", pid, tamanio);
}

// Función para destruir un proceso
void destruir_proceso(Memoria* memoria, int pid, int tamanio) {
    // Simulando la liberación de marcos asignados al proceso.
    for (int i = 0; i < tamanio; i++) {
        memoria->tabla_paginas->entradas[i].numeroDeMarco = -1; // Libera el marco
        memoria->tabla_paginas->entradas[i].bitDeValidez = 0; // Marca la página como inválida
    }

    log_info(logger, "Proceso %d destruido.", pid);
}
