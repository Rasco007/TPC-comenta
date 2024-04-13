#ifndef CPU_KERN_SER_H
#define CPU_KERN_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>


extern t_log* loggerError; 

void escucharAlKernel(); 

extern int socketCliente;

#endif