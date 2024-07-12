
/* - Las interfaces irán recibiendo desde Kernel distintas operaciones a realizar para determinado proceso, 
y le irán dando aviso a dicho módulo una vez completadas.
- Al iniciar una Interfaz de I/O la misma deberá recibir 2 parámetros:
	.Nombre: único dentro del sistema y servirá como identificación de la Interfaz
	.Archivo de Configuración*/

#include <main/entradasalida.h>

t_log* logger;
t_log* loggerError ;
t_config* config ;
int fd_kernel = 0;
int fd_memoria = 0;

char* TIPO_INTERFAZ ;
int TIEMPO_UNIDAD_TRABAJO ;
char* IP_KERNEL;
char* PUERTO_KERNEL ;
char* IP_MEMORIA ;
char* PUERTO_MEMORIA ;
char* PATH_BASE_DIALFS ;
int BLOCK_SIZE ;
int BLOCK_COUNT;
//t_bitarray *my_bitmap;

// yo lo corro con: ./bin/entradasalida IntX entradasalida.config
int main(int argc, char** argv) {

    if (argc < 3) {
    	fprintf(stderr, "Usar: %s IntX </home/utnso/tp-2024-1c-Grupo-SO-/entradasalida/io.config>\n", argv[0]);
    	return EXIT_FAILURE;
	}

    iniciar_io(argv); //abrir modulo con: ./bin/entradasalida /home/utnso/tp-2024-1c-Silver-Crime/entradasalida/entradasalida.config

	atexit(terminarPrograma); //cuando se haga exit, se llamara a terminarPrograma. esto es lo que hace atexit

	conexionMemoria();
	conexionKernel();
    // pruebas hardcodeadas
    create_bitmap_file("bitmap.dat", BLOCK_COUNT/8);
    create_bloques_file("bloques.dat", BLOCK_COUNT*BLOCK_SIZE);
    crearArchivo2("hola.txt");
    truncarArchivo2("hola.txt", BLOCK_SIZE*2);
    escribirCadenaEnArchivo("hola.txt", "1ERooO", 4); escribirCadenaEnArchivo("hola.txt", "hola", 0);
    crearArchivo2("chau.txt");
    truncarArchivo2("chau.txt", BLOCK_SIZE*2);
    escribirCadenaEnArchivo("chau.txt", "2DO000000000000", 0);
    crearArchivo2("otro.txt");
    truncarArchivo2("otro.txt", BLOCK_SIZE*2);
    escribirCadenaEnArchivo("otro.txt", "3ROoooo", 0);
    truncarArchivo2("hola.txt", BLOCK_SIZE*3);
    truncarArchivo2("chau.txt", BLOCK_SIZE*1);
    truncarArchivo2("otro.txt", BLOCK_SIZE*1);
    truncarArchivo2("otro.txt", BLOCK_SIZE*5);
    char *datoArchivo = leerDatosDesdeArchivo("hola.txt", 0, 4);
    printf("Dato de Archivo: %s\n", datoArchivo);
    free(datoArchivo);
    /*crearArchivo2("medio.txt");
    crearArchivo2("chau.txt");
    crearArchivo2("otro.txt");
    truncarArchivo2("otro.txt", BLOCK_SIZE*2);
    truncarArchivo2("medio.txt", BLOCK_SIZE*2);*/ // es necesario que el orden final sea por orden de creacion? osea hola->chau->otro->medio? porque queda hola->otro->chau->medio
    //nota: con este ejemplo, despues de compactar, el archivo "chau.txt" se posiciona despues de "otro.txt", esto esta bien?
    //delete_file("otro.txt");
    //crearArchivo2("nuevo.txt");
    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) io_atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);
    free(argv);
    return EXIT_SUCCESS;
}