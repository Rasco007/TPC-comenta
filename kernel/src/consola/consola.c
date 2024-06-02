#include <consola/consola.h>

int ejecutarConsola (int argc, char *argv[]) {
    logger = cambiarNombre(logger, "Consola");
    ejecutarScript(argv);
    return EXIT_SUCCESS;
}

//FUNCIONES POR CONSOLA

//EJECUTAR_SCRIPT [PATH]
void ejecutarScript(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        log_error(logger, "No se pudo abrir el archivo %s", path);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        
        // Check if line matches any function
        if (strcmp(line, "INICIAR_PROCESO") == 0) {
            iniciarProceso(line + strlen("INICIAR_PROCESO") + 1);
        } else if (strcmp(line, "FINALIZAR_PROCESO") == 0) {
            finalizarProceso(line + strlen("FINALIZAR_PROCESO") + 1);
        } else if (strcmp(line, "DETENER_PLANIFICACION") == 0) {
            detenerPlanificacion();
        } else if (strcmp(line, "INICIAR_PLANIFICACION") == 0) {
            iniciarPlanificacion();
        } else if (strcmp(line, "PROCESO_ESTADO") == 0) 
            procesoEstado();
        }
        fclose(file);
    }

//INICIAR_PROCESO[PATH]
void iniciarProceso(const char* path) {//Creo el pcb y lo ingreso a la cola de new
    t_pcb* pcb = crearPCB();
    ingresarANew(pcb);
} //Falta ver lo del path al FS que dice la consigna.

//FINALIZAR_PROCESO
void finalizarProceso(int pid){
    //TODO
}

//DETENER_PLANIFICACION
void detenerPlanificacion(){
    //TODO
}

//INICIAR_PLANIFICACION
void iniciarPlanificacion(){
    //TODO
}

//PROCESO_ESTADO
void procesoEstado(){
    log_info(logger, "Procesos en NEW");
    listarPIDS(pcbsNEW);
    log_info(logger, "Procesos en READY");
    listarPIDS(pcbsREADY);
    log_info(logger, "Procesos en EXEC");
    listarPIDS(pcbsEnMemoria);
}

