#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <includes/utils/utils.h>

int main(int argc, char* argv[]) {
    
    t_log* kernelLogger = iniciarKernelLogger();
    
    tKernelConfig* kernelConfig = leerKernelConfig(kernelLogger);
    if (kernelConfig == NULL) {
        liberarLogger(kernelLogger);
        return EXIT_FAILURE;        
    }
    
    log_info(kernelLogger, "Hola desde Kernel!!");
    terminarPrograma(kernelConfig, kernelLogger);
    return EXIT_SUCCESS;
}
