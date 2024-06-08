#include <consola/consola.h>
#include <readline/readline.h>
#include <readline/history.h>

//Se reciben dos archivos:El script con las funciones de kernel y el archivo de instrucciones
int ejecutarConsola () {
    char *linea;
    log_info(logger,"Consola iniciada. Por favor ingrese un comando");
    while (1) {
        linea = readline(">");
        if (!linea) {
            break;
        }
        if (linea) {
            add_history(linea);
            log_info(logger, "Comando ingresado: %s", linea);
        }
        if (!strncmp(linea, "exit", 4)) {
            free(linea);
            break;
        }
        if(!strncmp(linea, "INICIAR_PROCESO",15)){
            iniciarProceso("a");
        }
        if(!strncmp(linea, "FINALIZAR_PROCESO",17)){
            finalizarProceso(1);
        }
        if(!strncmp(linea, "DETENER_PLANIFICACION",21)){
            detenerPlanificacion();
        }
        if(!strncmp(linea, "INICIAR_PLANIFICACION",21)){
            iniciarPlanificacion();
        }
        if(!strncmp(linea, "MULTIPROGRAMACION", 17)) {
            int valor = atoi(linea + 17);
            modificarGradoMultiprogramacion(valor);
        }
        if(!strncmp(linea, "PROCESO_ESTADO", 14)) {
            procesoEstado();
        }

        free(linea);
    }

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
        if (!strcmp(line, "INICIAR_PROCESO") == 0) {
            iniciarProceso(line + strlen("INICIAR_PROCESO") + 1);
        } else if (!strcmp(line, "FINALIZAR_PROCESO") == 0) {
            //finalizarProceso(line + strlen("FINALIZAR_PROCESO") + 1);
        } else if (!strcmp(line, "DETENER_PLANIFICACION") == 0) {
            detenerPlanificacion();
        } else if (!strcmp(line, "INICIAR_PLANIFICACION") == 0) {
            iniciarPlanificacion();
        } else if (!strcmp(line, "PROCESO_ESTADO") == 0) 
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
    log_info(logger, "Busco");   
    t_pcb* pcb = buscarPID(pcbsNEW,pid); //TODO: esto deberia ser la cola de pcbsParaExit ! 
    log_info(logger, "Destruyo");   
    destruirPCB(pcb);
    log_info(logger, "Se finaliza el proceso <%d>", pid);    
}

//DETENER_PLANIFICACION
void detenerPlanificacion(){
    pausaPlanificacion=true;
    log_info(logger, "Planificacion detenida");
}

//INICIAR_PLANIFICACION
void iniciarPlanificacion(){
    if (pausaPlanificacion) {
        pausaPlanificacion=false;
        planificarALargoPlazo();
        planificarACortoPlazoSegunAlgoritmo();
        log_info(logger, "Planificaciones iniciadas");
    }
    //En caso que la planificación no se encuentre pausada, ignora el mensaje.
}

//PROCESO_ESTADO
void procesoEstado(){
    log_info(logger, "Procesos en NEW");
    listarPIDS(pcbsNEW);
    log_info(logger, "Procesos en READY");
    listarPIDS(pcbsREADY);
    log_info(logger, "Procesos en EXEC");
    listarPIDS(pcbsEnMemoria);

    //Hace falta?
    /*log_info(logger, "Procesos en BLOCK");
    listarPIDS(pcbsBloqueados);
    log_info(logger, "Procesos en EXIT");
    listarPIDS(pcbsParaExit);*/
}

//MULTIPROGRAMACION [VALOR]
void modificarGradoMultiprogramacion(int valor){
    //log_info(logger, "Recibo el valor: %d", valor); OJO SI RECIBE VALORES GRANDES POR EJEMPLO 999999999999 
    //TODO
}
