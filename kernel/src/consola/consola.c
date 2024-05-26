#include <consola/consola.h>

/*Cuando se crea un proceso por consola se le informa a memoria que debe 
  crear un proceso que se corresponda con las operaciones que se pasan en el archivo*/
int ejecutarConsola (int, char *archivos[]) {
    //TODO
    ejecutarScript(archivos[1]);
    return 0;
}

//FUNCIONES POR CONSOLA

//EJECUTAR_SCRIPT [PATH]
void ejecutarScript(const char* path) {
    FILE* archivo = fopen(path, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    
    fclose(archivo);
}

//INICIAR_PROCESO[PATH]
void iniciarProceso(const char* path) {
    //TODO
}

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
    //TODO
}