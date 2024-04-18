#include <includes/utils/utils.h>

void terminarPrograma(tCPUconfig* cpuConfig, t_log* cpuLogger);
void liberarMemoriaDinamica(tCPUconfig* cpuConfig, t_log* cpuLogger);
void liberarConfig(tCPUconfig* cpuConfig);
void liberarLogger(t_log* cpuLogger);

void terminarPrograma(tCPUconfig* cpuConfig, t_log* cpuLogger) {

    log_info(cpuLogger, "Fin de CPU");
    liberarMemoriaDinamica(cpuConfig, cpuLogger);
}

void liberarMemoriaDinamica(tCPUconfig* cpuConfig, t_log* cpuLogger) {

    liberarConfig(cpuConfig);
    liberarLogger(cpuLogger);
}

void liberarConfig(tCPUconfig* cpuConfig) {
    free(cpuConfig->ipMemoria);
}

void liberarLogger(t_log* cpuLogger) {
    log_destroy(cpuLogger);
}