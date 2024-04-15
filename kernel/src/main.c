#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <includes/utils/utils.h>

int main(int argc, char* argv[]) {
    
    t_log* kernelLogger = iniciarKernelLogger();
    if (kernelLogger == NULL) {
        return EXIT_FAILURE;        
    }

    tKernelConfig* kernelConfig = leerKernelConfig(kernelLogger);
    if (kernelConfig == NULL) {
        liberarLogger(kernelLogger);
        return EXIT_FAILURE;        
    }
    
    log_info(kernelLogger, "Hola desde Kernel!!");

    //start cliente de cpu dispatch
    int socketClientCPUDispatch = startCliente(kernelConfig->ipCpu, kernelConfig->puertoCpuDispatch, NOMBRE_CLIENTE_CPU_DISPATCH, tHsCode.HS_KERNEL, tHsCode.HS_CPU_DISPATCH);
    //start cliente de cpu interrupt
    int socketClientCPUInterrupt = startCliente(kernelConfig->ipCpu, kernelConfig->puertoCpuInterrupt, NOMBRE_CLIENTE_CPU_INTERRUPT, tHsCode.HS_KERNEL, tHsCode.HS_CPU_INTERRUPT);
    //start cliente de memoria
    int socketClientMemoria = startCliente(kernelConfig->ipMemoria, kernelConfig->puertoMemoria, NOMBRE_CLIENTE_MEMORIA, tHsCode.HS_KERNEL, tHsCode.HS_MEMORIA);

    terminarPrograma(kernelConfig, kernelLogger);
    
    return EXIT_SUCCESS;
}
