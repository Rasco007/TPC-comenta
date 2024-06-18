#include <conexionCPU/conexionCPU.h>

int tiempo;
MemoriaFisica *memoria;
//t_log *logger;
//t_config *config;
char* valorLeido; 

// Recibo peticiones de CPU y mando respuesta
int ejecutarServidorCPU(int *socketCliente) {
    tiempo = config_get_int_value(config, "RETARDO_RESPUESTA");

    while (1) {
        int peticion = recibirOperacion(*socketCliente);
        log_debug(logger, "Se recibió petición %d del CPU", peticion);

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
                log_info(logger, "Llegue al case MMU");
                recibirPeticionDeLectura(*socketCliente);
                
                break;
            case -1:
                log_error(logger, "El CPU se desconectó");
                return EXIT_FAILURE;
                break;
            default:
                log_warning(logger, "Operación desconocida del CPU.");
                break;
        }
    }
    return EXIT_SUCCESS;
}

char* leer(int32_t direccionFisica, int tamanio) {
    usleep(tiempo * 1000); // Simula el retardo

    char* punteroDireccionFisica = (char*)memoria->memoria + direccionFisica;
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
}

void escribir(char* valor, int32_t direccionFisica, int tamanio) {
    usleep(tiempo * 1000); // Simula el retardo

    char* punteroADirFisica = (char*)memoria->memoria + direccionFisica;
    
    memcpy(punteroADirFisica, valor, tamanio);

    free(valor);
}



