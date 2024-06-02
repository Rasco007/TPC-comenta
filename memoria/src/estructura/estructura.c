#include <estructura/estructura.h>
#include <commons/log.h>

extern t_log* logger;
extern t_log* loggerError;

// Función para traducir una dirección lógica a una dirección física
int traducirDireccionLogica(Memoria* memoria, int direccion_logica, int* marco, int* desplazamiento) {
    int tamano_pagina = memoria->tabla_paginas->numeroDeEntradas; // Tamaño de cada página
    int numero_pagina = direccion_logica / tamano_pagina;
    *desplazamiento = direccion_logica % tamano_pagina;

    if (numero_pagina >= memoria->tabla_paginas->numeroDeEntradas) {
        log_error(loggerError, "Direccion logica fuera de rango: %d\n", direccion_logica);
        return -1;
    }

    *marco = memoria->tabla_paginas->entradas[numero_pagina].numeroDeMarco;

    if (*marco == -1) {
        log_error(loggerError, "Pagina no asignada a un marco: %d\n", numero_pagina);
        return -1;
    }

    return 0;
}

// Función para crear la memoria
Memoria* crearMemoria(int num_paginas, int tamano_pagina) {

    log_info(logger, "Asignando Memoria...\n");

    Memoria* memoria = malloc(sizeof(Memoria));
    
    if (memoria == NULL) {
        log_error(loggerError, "Error al asignar memoria. (1)\n");
        return NULL;
    }

    memoria->memoria = malloc(num_paginas * tamano_pagina);

    if (memoria->memoria == NULL) {
        log_error(loggerError, "Error al asignar memoria. (2)\n");
        free(memoria);
        return NULL;
    }

    memoria->tabla_paginas = malloc(sizeof(TablaDePaginas));

    if (memoria->tabla_paginas == NULL) {
        log_error(loggerError, "Error al asignar memoria. (3)\n");
        free(memoria->memoria);
        free(memoria);
        return NULL;
    }

    memoria->tabla_paginas->numeroDeEntradas = num_paginas;
    memoria->tabla_paginas->entradas = malloc(num_paginas * sizeof(EntradaTablaDePaginas));
    
    if (memoria->tabla_paginas->entradas == NULL) {
        log_error(loggerError, "Error al asignar memoria. (4)\n");
        free(memoria->tabla_paginas);
        free(memoria->memoria);
        free(memoria);
        return NULL;
    }

    for (int i = 0; i < num_paginas; i++) {
        memoria->tabla_paginas->entradas[i].numeroDePagina = i;
        memoria->tabla_paginas->entradas[i].numeroDeMarco = -1; // -1 indica que el marco no está asignado
    }

    log_info(logger, "Memoria creada de manera exitosa!\n");

    return memoria;
}

// Función para destruir la memoria
void destruirMemoria(Memoria* memoria) {
    free(memoria->tabla_paginas->entradas);
    free(memoria->tabla_paginas);
    free(memoria->memoria);
    free(memoria);
}

// Función para escribir en memoria
void escribirMemoria(Memoria* memoria, int direccion_logica, void* datos, int tamano) {
    int marco, desplazamiento;
    if (traducirDireccionLogica(memoria, direccion_logica, &marco, &desplazamiento) == -1) {
        log_error(loggerError, "Error al traducir direccion logica: %d\n", direccion_logica);
        return;
    }

    int direccion_fisica = (marco * memoria->tabla_paginas->numeroDeEntradas) + desplazamiento;

    log_info(logger, "Escribiendo en direccion fisica: %d\n", direccion_fisica);

    memcpy((char*)memoria->memoria + direccion_fisica, datos, tamano);
    log_info(logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %d\n", 0, direccion_fisica, tamano);
}

// Función para leer de memoria
void* leerMemoria(Memoria* memoria, int direccion_logica, int tamano) {
    int marco, desplazamiento;
    if (traducirDireccionLogica(memoria, direccion_logica, &marco, &desplazamiento) == -1) {
        log_error(loggerError, "Error al traducir direccion logica: %d\n", direccion_logica);
        return NULL;
    }

    int direccion_fisica = (marco * memoria->tabla_paginas->numeroDeEntradas) + desplazamiento;

    log_info(logger, "Leyendo de direccion fisica: %d\n", direccion_fisica);

    void* buffer = malloc(tamano);
    memcpy(buffer, (char*)memoria->memoria + direccion_fisica, tamano);

    log_info(logger, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño: %d\n", 0, direccion_fisica, tamano);
    return buffer;
}

// Función para inicializar la tabla de páginas
TablaDePaginas* inicializarTablaDePaginas(int numeroDeEntradas) {
    TablaDePaginas* tabla = malloc(sizeof(TablaDePaginas));
    tabla->entradas = malloc(sizeof(EntradaTablaDePaginas) * numeroDeEntradas);
    tabla->numeroDeEntradas = numeroDeEntradas;
    for (int i = 0; i < numeroDeEntradas; i++) {
        tabla->entradas[i].numeroDePagina = i;
        tabla->entradas[i].numeroDeMarco = -1; // Al iniciar ninguna página está en un marco
        tabla->entradas[i].bitDeValidez = 0; // Al iniciar, ninguna página es válida
    }
    return tabla;
}

// Función para liberar la memoria utilizada por la tabla de páginas
void liberarTablaDePaginas(TablaDePaginas* tabla) {
    free(tabla->entradas);
    free(tabla);
}
