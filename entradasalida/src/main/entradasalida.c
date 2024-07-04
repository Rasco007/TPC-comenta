
/* - Las interfaces irán recibiendo desde Kernel distintas operaciones a realizar para determinado proceso, 
y le irán dando aviso a dicho módulo una vez completadas.
- Al iniciar una Interfaz de I/O la misma deberá recibir 2 parámetros:
	.Nombre: único dentro del sistema y servirá como identificación de la Interfaz
	.Archivo de Configuración*/

#include <main/entradasalida.h>
#define MAX_FILENAME_LENGTH 255
#define BITMAP_FILE "/home/utnso/tp-2024-1c-Silver-Crime/entradasalida/bitmap.dat"
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
t_bitarray *my_bitmap;

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
    sleep(1);
    crearArchivo2("hola");
   // crearArchivo2("chau");
    //truncarArchivo2("hola", 10);
    sleep(1);
    truncarArchivo2("hola", 64*7);
    truncarArchivo2("hola", 64);
    truncarArchivo2("hola", 64*2);
    delete_file("hola");
	pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) io_atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);
    return EXIT_SUCCESS;
}
//una funcion que reciba un nombre de archivo y un tamanio y lo trunque
void truncarArchivo2(char* nombre, int tamanio){
    char nombretxt[256];
    sprintf(nombretxt, "%s.txt", nombre);
    FILE *file = fopen(nombretxt, "r+"); // Abrir el archivo en modo lectura-escritura ('r+')
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }
    int cantidadBloques;
    if (tamanio>BLOCK_SIZE) {
        cantidadBloques = tamanio/BLOCK_SIZE;
        //redondeo para arriba
        if (tamanio%BLOCK_SIZE!=0) {
            cantidadBloques++;
        }
    }
    else {
        cantidadBloques = 1;
    }
    printf("nueva cantidadBloques: %d\n", cantidadBloques);
    // abrir su metadata para obtener bloque inicial y escribir el nuevo tamanio
    char nombreMetadata[256];
    sprintf(nombreMetadata, "%s.metadata", nombre);
    FILE *fileMetadata = fopen(nombreMetadata, "rw");
    if (fileMetadata == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }
    t_config *config = config_create(nombreMetadata);
    if (config == NULL) {
        perror("Error al abrir el archivo de metadata");
        fclose(fileMetadata);
        return;
    }
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    printf("bloqueInicial: %d\n", bloqueInicial);
    //tamanio del archivo en bytes
    int tamanoArchivo = config_get_int_value(config, "tamanoArchivo");
    printf("tamanoArchivo: %d\n", tamanoArchivo);
    int bloquesarchivo= tamanoArchivo/BLOCK_SIZE;
           if (tamanio%BLOCK_SIZE!=0) {
            bloquesarchivo++;
        }
    // Abrir el archivo bitmap.dat
    const char *filename = "bitmap.dat";
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo bitmap.dat");
        return;
    } 
    // Mapear el archivo bitmap.dat en memoria
    void *bitmap = mmap(NULL, 1024/8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return;
    }
    t_bitarray *my_bitmap2 = bitarray_create_with_mode(bitmap, 1024/8 * CHAR_BIT, MSB_FIRST);
    //int primerBloqueLibre =obtenerPrimeraPosicionLibre(my_bitmap2);
    //bitarray_set_bit(my_bitmap2, primerBloqueLibre);
    if (msync(bitmap, 1024/8, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo bitmap.dat");
        munmap(bitmap, 1024/8);
        close(fd);
        return ;
    }
    //si el nuevo tamanio es menor al anterior, se liberan bloques
    if (tamanio<tamanoArchivo) {
        printf("tamanio<tamanoArchivo\n");
        // Calcular la cantidad de bloques ocupados por el archivo
        int bloquefinal = bloqueInicial+cantidadBloques;
        // Liberar los bloques ocupados por el archivo
        for (int i = bloquesarchivo; i >bloquefinal; i--) {
            int posicionBit = obtenerPrimeraPosicionLibre(my_bitmap2)-1;
            // Liberar el bit correspondiente en el bitarray
            bitarray_clean_bit(my_bitmap2, posicionBit);
        }
        //modificar metadata
        escribir_metadata(nombre, bloqueInicial, tamanio);
    }
    //si el nuevo tamanio es mayor al anterior, se buscan bloques libres y se los asigna
    else if (tamanio>tamanoArchivo) {
        printf("tamanio>tamanoArchivo\n");
        // Calcular la cantidad de bloques ocupados por el archivo
        int bloquefinal = bloqueInicial+cantidadBloques;
        // Asignar los bloques libres al archivo
        printf("bloquefinal: %d\n", bloquefinal);
        for (int i = bloqueInicial+1; i <= bloquefinal-1; i++) {
            int posicionBit = obtenerPrimeraPosicionLibre(my_bitmap2);
            bitarray_set_bit(my_bitmap2, posicionBit);
        }
        //modificar metadata
        escribir_metadata(nombre, bloqueInicial, tamanio);
    }
    if (msync(bitmap, 1024/8, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo bitmap.dat");
    }
    // Liberar recursos
    munmap(bitmap, 1024/8);
    close(fd);
}

void delete_file(const char *nombre) {
    char nombretxt[256];
    sprintf(nombretxt, "%s.txt", nombre);
    // Eliminar el archivo
    if (remove(nombretxt) != 0) {
        perror("Error al borrar el archivo");
        return;
    }
    // Abrir el archivo bitmap.dat
    const char *filename = "bitmap.dat";
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo bitmap.dat");
        return;
    } 
    // Mapear el archivo bitmap.dat en memoria
    void *bitmap = mmap(NULL, 1024/8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return;
    }
    // Crear el bitarray a partir del bitmap mapeado
    t_bitarray *my_bitmap2 = bitarray_create_with_mode(bitmap, 1024/8 * CHAR_BIT, MSB_FIRST);
    // abrir su metadata como lectura para obtener bloque inicial y tamanio
    char nombreMetadata[256];
    sprintf(nombreMetadata, "%s.metadata", nombre);
    FILE *fileMetadata = fopen(nombreMetadata, "r");
    if (fileMetadata == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }
    t_config *config = config_create(nombreMetadata);
    if (config == NULL) {
        perror("Error al abrir el archivo de metadata");
        fclose(fileMetadata);
        return;
    }
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    printf("bloqueInicial: %d\n", bloqueInicial);
    //tamanio del archivo en bytes
    int tamanoArchivo = config_get_int_value(config, "tamanoArchivo");
    // Calcular la cantidad de bloques ocupados por el archivo
    int bloquesOcupados= tamanoArchivo/BLOCK_SIZE;
    if (tamanoArchivo==0)
        bloquesOcupados=1;
    printf("bloquesOcupados: %d\n", bloquesOcupados);
    // Liberar los bloques ocupados por el archivo
    /* for (int i = bloqueInicial; i < bloqueInicial+ bloquesOcupados; i++) {
        int posicionBit = bloqueInicial + i;
        // Liberar el bit correspondiente en el bitarray
        bItarray_clean_bit(my_bitmap2, posicionBit);
    }*/
   // bitarray_clean_bit(my_bitmap2, bloqueInicial); 
            // Calcular la cantidad de bloques ocupados por el archivo
        //int bloquefinal = bloqueInicial+cantidadBloques;
        // Liberar los bloques ocupados por el archivo
        for (int i = bloquesOcupados; i >=0; i--) {
            int posicionBit = obtenerPrimeraPosicionLibre(my_bitmap2)-1;
            // Liberar el bit correspondiente en el bitarray
            bitarray_clean_bit(my_bitmap2, posicionBit);
        }
        //modificar metadata
    // Sincronizar los cambios en el archivo bitmap.dat
    if (msync(bitmap, 1024/8, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo bitmap.dat");
    }
    // Liberar recursos
    munmap(bitmap, 1024/8);
    close(fd);
    printf("Archivo %s borrado exitosamente y bit liberado en el bitmap.\n", nombre);
}

void crearArchivo2(char* nombre) {
    char nombretxt[256];
    sprintf(nombretxt, "%s.txt", nombre);
    FILE *file = fopen(nombretxt, "w"); // Abrir el archivo en modo escritura ('w')
    if (file == NULL) {
        perror("Error al crear el archivo");
        return;
    }
    // Crear el archivo de metadata
    /*char nombreMetadata[256];
    sprintf(nombreMetadata, "%s.metadata", nombre); // Nombre del archivo metadata
    FILE *fileMetadata = fopen(nombreMetadata, "w");
    if (fileMetadata == NULL) {
        perror("Error al crear el archivo de metadata");
        fclose(file);
        return;
    }
    char * pathArchivo = string_from_format ("main/%s.fcb", nombreMetadata);*/
    /*if (access (pathArchivo, F_OK)) {
        free (pathArchivo); 
        return ;
    }*/
    
    /*t_config * archivo = config_create (pathArchivo);
    if (!archivo) {
        free (pathArchivo);
        return ;
    }
    config_set_value (archivo, "BLOQUE_INICIAL", "0");
    config_set_value (archivo, "TAMANIO_ARCHIVO", "0");
    */
   
    const char *filename = "bitmap.dat";
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo bitmap.dat");
        return ;
    } 
    void *bitmap = mmap(NULL, 1024/8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return ;
    }
    t_bitarray *my_bitmap2 = bitarray_create_with_mode(bitmap, 1024/8 * CHAR_BIT, MSB_FIRST);
    int primerBloqueLibre =obtenerPrimeraPosicionLibre(my_bitmap2);
    bitarray_set_bit(my_bitmap2, primerBloqueLibre);
    if (msync(bitmap, 1024/8, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo bitmap.dat");
        munmap(bitmap, 1024/8);
        close(fd);
        return ;
    }
    escribir_metadata(nombre, primerBloqueLibre, 0);
    printf("Archivo %s creado exitosamente con tamaño 0 bytes.\n", nombre);
    munmap(bitmap, 1024/8);
    close(fd);
}

// Estructura para almacenar la asociación entre archivo y bit en el bitarray
int obtenerPrimeraPosicionLibre(t_bitarray *bitmap) {
    int posicion = -1;
    int size = bitmap->size;
    // Iterar sobre cada bit del bitmap
    for (int i = 0; i < size; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            posicion = i;
            break;  // Salir del bucle al encontrar el primer bit libre
        }
    }
    return posicion;
}

void create_bitmap_file(const char *filename, size_t size) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        // El archivo no existe, lo creamos
        fd = open(filename, O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            perror("Error al crear el archivo bitmap.dat");
            return;
        }
        // Establecer el tamaño del archivo
        if (ftruncate(fd, size) == -1) {
            perror("Error al establecer el tamaño del archivo bitmap.dat");
            close(fd);
            return;
        }
    }
    // Mapear el archivo en memoria
    void *bitmap = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return;
    }
    // Crear el t_bitarray para manejar el bitmap
    t_bitarray *my_bitmap = bitarray_create_with_mode(bitmap, size * CHAR_BIT, MSB_FIRST);
    if (my_bitmap == NULL) {
        perror("Error al crear el bitarray para el bitmap");
        munmap(bitmap, size);
        close(fd);
        return;
    }
    // Ejemplo: Marcar algunos bloques como ocupados
    /*for (int i = 0; i < 8; i++) {
        bitarray_set_bit(my_bitmap, i);
        printf("Bloque %d está ocupado.\n", i);
    }
    // Ejemplo: Verificar si un bloque está ocupado
    if (bitarray_test_bit(my_bitmap, 8)) 
        printf("Bloque 8 está ocupado.\n");
    else 
        printf("Bloque 8 está libre.\n");*/
    // Destruir el bitarray y liberar recursos
    bitarray_destroy(my_bitmap);
    munmap(bitmap, size);
    close(fd);
}

void escribir_metadata(char *nombre, int bloqueInicial, int tamanoArchivo) {
    // Construir el nombre del archivo de metadata
    char nombreMetadata[256];
    sprintf(nombreMetadata, "%s.metadata", nombre);
        // Verificar si el archivo de metadata existe
    FILE *fileMetadata = fopen(nombreMetadata, "r");
    if (fileMetadata == NULL) {
        // Si el archivo no existe, crearlo en modo escritura
        fileMetadata = fopen(nombreMetadata, "w");
        if (fileMetadata == NULL) {
            perror("Error al crear el archivo de metadata");
            return;
        }
        fclose(fileMetadata);
        // Volver a abrirlo en modo de lectura-escritura para utilizarlo con t_config
        fileMetadata = fopen(nombreMetadata, "r+");
        if (fileMetadata == NULL) {
            perror("Error al abrir el archivo de metadata");
            return;
        }
    } else {
        fclose(fileMetadata);
    }
    // Crear un archivo de configuración para el metadata
    t_config *config = config_create(nombreMetadata);
    if (config == NULL) {
        perror("Error al crear el archivo de metadata");
        return;
    }
    // Convertir los valores a strings y guardarlos en el archivo de metadata
    char valorBloqueInicial[20];
    char valorTamanoArchivo[20];
    sprintf(valorBloqueInicial, "%d", bloqueInicial);
    sprintf(valorTamanoArchivo, "%d", tamanoArchivo);
    config_set_value(config, "bloqueInicial", valorBloqueInicial);
    config_set_value(config, "tamanoArchivo", valorTamanoArchivo);
    // Guardar los cambios en el archivo
    if (config_save(config) == -1) {
        perror("Error al guardar el archivo de metadata");
    }
    // Liberar recursos
    config_destroy(config);
    printf("Metadata para %s creada exitosamente.\n", nombre);
}