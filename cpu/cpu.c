#include <stdlib.h>
#include <stdio.h>
#include <includes/logger/logger.h>
#include <includes/config/config.h>
//programa principal-cpu


int main(void){
	t_log* cpuLogger=iniciarCPULogger();
	tCPUconfig* cpuConfig=leerConfig(cpuLogger); 
	
	return 0;
}	