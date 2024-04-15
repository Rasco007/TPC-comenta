#include <includes/logger/logger.h>
#include <includes/config/config.h>
#include <includes/utils/utils.h>

bool sonSocketsValidos(int socketClientCPUDispatch, int socketClientCPUInterrupt, int socketClientMemoria);

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
    int socketClientCPUDispatch = startCliente(kernelConfig->ipCpu, kernelConfig->puertoCpuDispatch, NOMBRE_CLIENTE_CPU_DISPATCH, HS_KERNEL, HS_CPU_DISPATCH, kernelLogger);
    //start cliente de cpu interrupt
    int socketClientCPUInterrupt = startCliente(kernelConfig->ipCpu, kernelConfig->puertoCpuInterrupt, NOMBRE_CLIENTE_CPU_INTERRUPT, HS_KERNEL, HS_CPU_INTERRUPT, kernelLogger);
    //start cliente de memoria
    int socketClientMemoria = startCliente(kernelConfig->ipMemoria, kernelConfig->puertoMemoria, NOMBRE_CLIENTE_MEMORIA, HS_KERNEL, HS_MEMORIA, kernelLogger);

    if (!sonSocketsValidos(socketClientCPUDispatch, socketClientCPUInterrupt, socketClientMemoria)) {
        terminarPrograma(kernelConfig, kernelLogger);
        return EXIT_FAILURE;
    }

    terminarPrograma(kernelConfig, kernelLogger);
    
    return EXIT_SUCCESS;
}

bool sonSocketsValidos(int socketClientCPUDispatch, int socketClientCPUInterrupt, int socketClientMemoria) {
    return socketClientCPUDispatch != -1 && socketClientCPUInterrupt != -1 && socketClientMemoria != -1;
}
