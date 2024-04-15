#include <includes/utils/utils.h>

void liberarMemoriaDinamica(tKernelConfig* kernelConfig, t_log* kernelLogger);
void liberarConfig(tKernelConfig* kernelConfig);
void liberarLogger(t_log* kernelLogger);

void terminarPrograma(tKernelConfig* kernelConfig, t_log* kernelLogger) {

    log_info(kernelLogger, "Fin de kernel");
    liberarMemoriaDinamica(kernelConfig, kernelLogger);
}

void liberarMemoriaDinamica(tKernelConfig* kernelConfig, t_log* kernelLogger) {

    liberarConfig(kernelConfig);
    liberarLogger(kernelLogger);
}

void liberarConfig(tKernelConfig* kernelConfig) {

    free(kernelConfig->ipCpu);
    free(kernelConfig->ipMemoria);
    list_destroy_and_destroy_elements(kernelConfig->recursos, free);
    list_destroy_and_destroy_elements(kernelConfig->instanciasRecursos, free);
}

void liberarLogger(t_log* kernelLogger) {
    log_destroy(kernelLogger);
}