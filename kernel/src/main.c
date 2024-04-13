#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>

int main(int argc, char* argv[]) {
    decir_hola("Kernel");
    t_log* kernelLogger = iniciarKernelLogger();
    tKernelConfig* kernelConfig = leerKernelConfig(kernelLogger);
    return 0;
}
