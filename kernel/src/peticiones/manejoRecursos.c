#include <peticiones/manejoRecursos.h>
#include <peticiones/syscalls.h>


int cantidadRecursos;

// crear colas de bloqueo
void crearColasBloqueo()
{
	instanciasRecursos = NULL; 
	recursos = list_create();
    nombresRecursos = obtenerRecursos(); //[RA,RB,RC...]
    char **cantInstanciasRecursos = obtenerInstanciasRecursos();//[1,2,1...]
    int tamanio = tamanioArrayCharDoble(cantInstanciasRecursos);

    cantidadRecursos=tamanio;
    for (int i = 0; i < tamanio; i++)
    {
        int instanciasConvertEntero = atoi(cantInstanciasRecursos[i]);
        instanciasRecursos=realloc(instanciasRecursos,(i+1)*sizeof(int));
        instanciasRecursos[i]=instanciasConvertEntero;

        t_list *colaBloqueo = list_create();
        list_add(recursos, colaBloqueo);
    }

    destruirArrayCharDoble(cantInstanciasRecursos);
}

// Devuelve el indice que se corresponde al recurso correspondiente, -1 si no lo encuentra
int indiceRecurso (char * recurso){
    int tamanio = tamanioArrayCharDoble(nombresRecursos);

    for (int i = 0; i < tamanio; i++)
        if (!strcmp(recurso, nombresRecursos[i]))
            return i;
    return -1;
}

void liberarColasBloqueo(){
	destruirInstanciasRecursos();
	destruirArrayCharDoble(nombresRecursos);
	destruirRecursos();
}

//Destruccion de colas de bloqueo
void destruirInstanciasRecursos(){
	free(instanciasRecursos);
}


void destruirArrayCharDoble (char ** array){
	int tamanio=tamanioArrayCharDoble(array);
	for (int i = 0; i<tamanio; i++) free(array[i]);
	free(array);
}


void colaBloqueadosDestroyer(void* colaBloqueados){
	list_destroy_and_destroy_elements(colaBloqueados,(void*)destruirPCB);
}

void destruirRecursos(){
	list_destroy_and_destroy_elements(recursos, colaBloqueadosDestroyer);
}

//Calcula el tamanio de un array char doble que vienen de los config y termina en NULL
int tamanioArrayCharDoble (char **arreglo){  
	    return string_array_size(arreglo);
}

void liberarRecursosAsignados(t_pcb* proceso){
    int cantRecursos = list_size(proceso->recursosAsignados);
    //log_error(logger,"La cant de recursos es %d",(cantRecursos));
    int i;
    if(cantRecursos!=0){
        for(i=0; i<cantRecursos;cantRecursos--){
            debug ("%d", i);
            char * parametros[3] = {(char *)list_get(proceso->recursosAsignados, i), "", "EXIT"}; 
            signal_s(proceso, parametros);
        }
    }

    for(i=0;i<list_size(recursos);i++){
        t_list* colaBloquadosRecurso=list_get(recursos,i);
        for(int j=0;j<list_size(colaBloquadosRecurso);j++){
            t_pcb* pcbBloqueado=list_get(colaBloquadosRecurso,j);
            if(pcbBloqueado->pid==proceso->pid){
                instanciasRecursos[i]++;
                list_remove(colaBloquadosRecurso,j);
                //log_warning(logger,"Se elimina el pid <%d> de las colas de bloqueados",proceso->pid);
            }
        }
    }
}

void eliminarRecursoLista(t_list* recursos, char* recurso){
    int cantRecursos = list_size(recursos);
    //log_warning(logger,"La cant de recursos es %d",(cantRecursos));
    //log_warning(logger,"el recurso es %s",(recurso));
    int i;
    
    for(i=0;i<cantRecursos;i++){ 
        //log_warning(logger,"Mi lista tiene %s",(char*)list_get(recursos, i));
        
        if(!strcmp((char*)list_get(recursos,i), recurso)){
            list_remove(recursos,i); //SI HAY LEAK DEMAS ,CAMBIARLO POR REEEEMOVE AND DESTROY
            //int cantidadRecursos = list_size(recursos);
            //log_warning(logger,"La cant de recursos es %d",(cantidadRecursos));
            return;  
        }
    }
}