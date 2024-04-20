#include <includes/utils/utils.h>

void liberarConfig(tMemoriaConfig* memoriaConfig);

void terminarPrograma(tMemoriaConfig* memoriaConfig, t_log* memoriaLogger) {
    log_info(memoriaLogger, "Fin de memoria");
}

void liberarConfig(tMemoriaConfig* memoriaConfig) {
    free(memoriaConfig->puertoEscucha);
    free(memoriaConfig->pathInstrucciones);
}

void liberarLogger(t_log* memoriaLogger) {
    log_destroy(memoriaLogger);
}