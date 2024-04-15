#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>

int main(int argc, char* argv[]) {
    t_log* kernelLogger = iniciarKernelLogger();
    tKernelConfig* kernelConfig = leerKernelConfig(kernelLogger);
    log_info(kernelLogger, "Hola desde Kernel!!");
    return 0;
}
