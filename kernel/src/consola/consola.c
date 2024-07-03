#include <consola/consola.h>
#include <readline/readline.h>
#include <readline/history.h>

void str_to_upper(char *str) {
    for(int i = 0; str[i]!= '\0'; i++) {
        if(str[i] == ' ') break; // Salta si encuentra un espacio
        str[i] = toupper((unsigned char)str[i]); // Convierte a mayúscula
    }
}

//Se reciben dos archivos:El script con las funciones de kernel y el archivo de instrucciones
int ejecutarConsola () {
    char *linea;
    logger=cambiarNombre(logger,"Consola-Kerel");
    log_info(logger,"Consola iniciada. Por favor ingrese un comando. Puede ingresar MENU para ver los comandos disponibles.");
    while (1) {
        linea = readline(">");
        str_to_upper(linea);
        if (!linea) {
            break;
        }
        if (!strncmp(linea, "EXIT", 4)) {
            free(linea);
            exit(EXIT_SUCCESS);
            break;
        }
        //Si escribo los comandos....
        if(!strncmp(linea, "EJECUTAR_SCRIPT", 14)){
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                char* path = token;
                ejecutarScript(path);
            } else {
                log_error(logger, "No se proporcionó un path para EJECUTAR_SCRIPT");
            }
        }
        if(!strncmp(linea, "I",1)){
            iniciarProceso("src/scripts_memoria/PLANI_1");
            /*char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                char* path = token;
                iniciarProceso(path);
            } else {
                log_error(logger, "No se proporcionó un path para INICIAR_PROCESO");
            }*/
        }
        if(!strncmp(linea, "FINALIZAR_PROCESO",17)){
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int pid = atoi(token); // Convierte el token en un número entero
                finalizarProceso(pid);
            } else {
                log_error(logger, "No se proporcionó un PID para FINALIZAR_PROCESO");
            }
        }
        if(!strncmp(linea, "DETENER_PLANIFICACION",21)){
            detenerPlanificacion();
        }
        if(!strncmp(linea, "INICIAR_PLANIFICACION",21)){
            iniciarPlanificacion();
        }
        if(!strncmp(linea, "MULTIPROGRAMACION", 17)) {
            char *token = strtok(linea, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int valor = atoi(token); // Convierte el token en un número entero
                modificarGradoMultiprogramacion(valor);
            } else {
                log_error(logger, "No se proporcionó un valor para MULTIPROGRAMACION");
            }
        }
        if(!strncmp(linea, "PROCESO_ESTADO", 14)) {
            procesoEstado();
        }
        if(!strncmp(linea,"MENU",strlen("MENU"))){
            log_info(logger,"EJECUTAR_SCRIPT [PATH] - Ejecuta un script con comandos de kernel\n"
                            "INICIAR_PROCESO [PATH] - Inicia un proceso\n"
                            "FINALIZAR_PROCESO [PID] - Finaliza un proceso\n"
                            "DETENER_PLANIFICACION - Detiene la planificacion\n"
                            "INICIAR_PLANIFICACION - Inicia la planificacion\n"
                            "MULTIPROGRAMACION [VALOR] - Modifica el grado de multiprogramacion\n"
                            "PROCESO_ESTADO - Muestra el estado de los procesos\n"
                            "MENU - Muestra los comandos disponibles\n"
                            "EXIT - Sale de la consola");
        }

        free(linea);
    }

    return EXIT_SUCCESS;
}

//FUNCIONES POR CONSOLA

//EJECUTAR_SCRIPT [PATH]
void ejecutarScript(const char* path) {
    FILE* file = fopen(path, "r"); //Abro el archibo en modo lectura
    if (file == NULL) {
        log_error(logger, "No se pudo abrir el archivo %s", path);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        
        if (!strncmp(line, "INICIAR_PROCESO", strlen("INICIAR_PROCESO"))) {
            char *token = strtok(line, " ");
            token = strtok(NULL, " "); 
            if (token != NULL) {
                iniciarProceso(token);
            } else {
                log_error(logger, "No se proporcionó un argumento para INICIAR_PROCESO");
            }
        } 
        else if(!strncmp(line, "FINALIZAR_PROCESO", strlen("FINALIZAR_PROCESO"))) {
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int pid = atoi(token); // Convierte el token en un número entero
                finalizarProceso(pid);
            } else {
                log_error(logger, "No se proporcionó un argumento para FINALIZAR_PROCESO");
            }
        } 
        else if(!strncmp(line, "DETENER_PLANIFICACION", strlen("DETENER_PLANIFICACION"))) {
            detenerPlanificacion();
        } 
        else if(!strncmp(line, "INICIAR_PLANIFICACION", strlen("INICIAR_PLANIFICACION"))) {
            iniciarPlanificacion();
        } 
        else if(!strncmp(line, "MULTIPROGRAMACION", strlen("MULTIPROGRAMACION"))) {
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                int valor = atoi(token); // Convierte el token en un número entero
                modificarGradoMultiprogramacion(valor);
            } else {
                log_error(logger, "No se proporcionó un argumento para MULTIPROGRAMACION");
            }
        } 
        else if(!strncmp(line, "PROCESO_ESTADO", strlen("PROCESO_ESTADO"))) {
            procesoEstado();
        } 
        else {
            log_error(logger, "Comando no reconocido: %s", line);
        }
    }
    fclose(file);
}

//INICIAR_PROCESO[PATH]
void iniciarProceso(const char* path) {//Creo el pcb y lo ingreso a la cola de new
    t_pcb* pcb = crearPCB(path);
    ingresarANew(pcb);
    //enviarPathDeInstrucciones(path);
} 

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
    imprimirListaPCBs(pcbsNEW);
    log_info(logger, "Procesos en READY");
    imprimirListaPCBs(pcbsREADY);
    log_info(logger, "Procesos en EXEC");
    imprimirListaPCBs(pcbsEnMemoria);
    log_info(logger, "Procesos en BLOCKED");
    listarPIDS(pcbsBloqueados);
    log_info(logger, "Procesos en EXIT");
    listarPIDS(pcbsParaExit);
}

//MULTIPROGRAMACION [VALOR]
void modificarGradoMultiprogramacion(int valor){
    if(valor<=0 || valor>=15){
        log_error(logger, "Multiprogramacion no permitida. Ingrese un valor entre 1 y 14.");
        return;
    } else{
        gradoMultiprogramacion = valor;
        log_info(logger, "Grado de multiprogramacion modificado a %d", valor);
    }
}
