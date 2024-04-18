#include <includes/utils/utils.h>

void liberarMemoriaDinamica(tInterfazConfig* interfazConfig, t_log* interfazLogger);
void liberarConfig(tInterfazConfig* interfazConfig);

void terminarPrograma(tInterfazConfig* interfazConfig, t_log* interfazLogger) {

    log_info(interfazLogger, "Fin de interfaz");
    liberarMemoriaDinamica(interfazConfig, interfazLogger);
}

void liberarMemoriaDinamica(tInterfazConfig* interfazConfig, t_log* interfazLogger) {

    liberarConfig(interfazConfig);
    liberarLogger(interfazLogger);
}

void liberarConfig(tInterfazConfig* interfazConfig) {

    free(interfazConfig->ip_kernel);
    free(interfazConfig->ip_memoria);

}

void liberarLogger(t_log* interfazLogger) {
    log_destroy(interfazLogger);
} 