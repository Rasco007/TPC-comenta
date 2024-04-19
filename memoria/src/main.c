#include <stdlib.h>
#include <stdio.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>

int main(int argc, char* argv[]) {
    t_log* memoriaLogger = iniciarMemoriaLogger();
    if (memoriaLogger == NULL){
        return EXIT_FAILURE;
    }

    tMemoriaConfig* memoriaConfig = leerMemoriaConfig(memoriaLogger);
    if (memoriaConfig == NULL) {
        liberarLogger(memoriaLogger);
        return EXIT_FAILURE;
    }
    
    log_info(memoriaLogger, "¡Saludos desde la Memoria!");

    return 0;
}
