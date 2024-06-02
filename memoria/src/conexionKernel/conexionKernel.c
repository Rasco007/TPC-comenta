#include <conexionKernel/conexionKernel.h>

t_peticion *peticion;
int cantidadMaximaPaginas;
uint32_t direccionBasePagina;
uint32_t tamanioPagina;

int ejecutarServidorKernel(int *socketCliente){
	cantidadMaximaPaginas = config_get_int_value(config, "TAM_PAGINA");
	while (1)
	{
		int peticionRealizada = recibirOperacion(*socketCliente);
		switch (peticionRealizada)
		{
		case NEWPCB:
			t_proceso *procesoNuevo = crearProcesoEnMemoria(recibirPID(*socketCliente));
			//enviarTablaPaginas(procesoNuevo);
			break;
		case ENDPCB:
			uint32_t pid = recibirPID(*socketCliente);
			//liberarTodosLasPaginas(pid);
			eliminarProcesoDeMemoria (pid);
			log_info(logger, "Eliminación de Proceso PID: <%d>", pid);
			break;
		//Falta creacion y eliminacion de paginas
		case -1:
			log_error(logger, "El Kernel se desconecto");
			return EXIT_FAILURE;
			break;
		default:
			log_warning(logger, "Operacion desconocida: %d ", peticionRealizada);
			break;
		}
	}
}

void procesarResultado(int resultado, int socketKernel){
	return 0;
}

t_proceso *crearProcesoEnMemoria(int pid){

	t_proceso *procesoNuevo = malloc(sizeof(t_proceso));
	procesoNuevo->pid = pid;
	//procesoNuevo->TablaDePaginas = crearTablaDePaginas(pid);
	//list_add(, (void *)procesoNuevo);
	log_info(logger, "Creacion de Proceso PID: <%d>", procesoNuevo->pid);

	return procesoNuevo;
}

void eliminarProcesoDeMemoria(t_proceso *proceso){
	//list_remove_element (, (void *) proceso);
	list_destroy_and_destroy_elements(proceso->TablaDePaginas, free);
	free(proceso);
}