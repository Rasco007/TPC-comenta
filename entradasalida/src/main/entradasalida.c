
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
#define RETRASO_COMPACTACION 50000
#define PATH_BASE_DIALFS "/home/utnso/tp-2024-1c-Silver-Crime/entradasalida/src/archivosFS"
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
    escribirCadenaEnArchivo("hola.txt", "1ERooO", 0);
    crearArchivo2("chau.txt");
    truncarArchivo2("chau.txt", BLOCK_SIZE*2);
    escribirCadenaEnArchivo("chau.txt", "2DO000000000000", 0);
    crearArchivo2("otro.txt");
    truncarArchivo2("otro.txt", BLOCK_SIZE*2);
    escribirCadenaEnArchivo("otro.txt", "3ROoooo", 0);
    truncarArchivo2("hola.txt", BLOCK_SIZE*3);
    truncarArchivo2("chau.txt", BLOCK_SIZE*1);
    truncarArchivo2("otro.txt", BLOCK_SIZE*1);
    /*crearArchivo2("medio.txt");
    crearArchivo2("chau.txt");
    crearArchivo2("otro.txt");
    truncarArchivo2("otro.txt", BLOCK_SIZE*2);
    truncarArchivo2("medio.txt", BLOCK_SIZE*2); */// es necesario que el orden final sea por orden de creacion? osea hola->chau->otro->medio? porque queda hola->otro->chau->medio
    //nota: con este ejemplo, despues de compactar, el archivo "chau.txt" se posiciona despues de "otro.txt", esto esta bien?
    //delete_file("otro.txt");
    //crearArchivo2("nuevo.txt");
    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) io_atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);
    free(argv);
    return EXIT_SUCCESS;
}

//una funcion que reciba un nombre de archivo y un tamanio y lo trunque
void truncarArchivo2(char* nombre, int tamanio){
    usleep(TIEMPO_UNIDAD_TRABAJO*1000);
    log_info(logger, "Truncar archivo: <%s> Tamaño: %d bytes", nombre, tamanio);
    char pathArchivo[256] = PATH_BASE_DIALFS;
    sprintf(pathArchivo, "%s/%s", PATH_BASE_DIALFS, nombre);
    FILE *file = fopen(pathArchivo, "r+"); // Abrir el archivo en modo lectura-escritura ('r+')
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }
    int cantidadBloques;
    if (tamanio>BLOCK_SIZE) {
        cantidadBloques = tamanio/BLOCK_SIZE;
        if (tamanio%BLOCK_SIZE!=0) //redondeo para arriba
            cantidadBloques++;
    }
    else 
        cantidadBloques = 1;
    //printf("nueva cantidadBloques: %d\n", cantidadBloques);
    // abrir su metadata para obtener bloque inicial y escribir el nuevo tamanio
    char pathMetadata[128];
    char *nombreSinExtension = obtenerNombreSinExtension(nombre);
    sprintf(pathMetadata, "%s/%s.metadata",PATH_BASE_DIALFS, nombreSinExtension);
    char pathMetadatabeta[128];
    sprintf(pathMetadatabeta, "%s.metadata", nombreSinExtension);
    t_config *config = config_create(pathMetadata);
    if (config == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    //tamanio del archivo en bytes
    int tamanoArchivo = config_get_int_value(config, "tamanoArchivo");
    config_destroy(config);
    //printf("bloqueInicial: %d\n", bloqueInicial);printf("tamanoArchivo: %d\n", tamanoArchivo);
    int bloquesarchivo= tamanoArchivo/BLOCK_SIZE;
    if (tamanoArchivo%BLOCK_SIZE!=0) 
        bloquesarchivo++;
    if (tamanoArchivo==0)
        bloquesarchivo=1;   
    int bloquefinal = bloqueInicial+bloquesarchivo-1;
    //printf("bloquesarchivo: %d\n", bloquesarchivo); 
    // Abrir el archivo bitmap.dat
    char pathBitmap[256];
    sprintf(pathBitmap, "%s/bitmap.dat", PATH_BASE_DIALFS);
    int fd = open(pathBitmap, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo bitmap.dat");
        return;
    } 
    // Mapear el archivo bitmap.dat en memoria
    void *bitmap = mmap(NULL, BLOCK_COUNT/8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return;
    }
    t_bitarray *my_bitmap2 = bitarray_create_with_mode(bitmap, BLOCK_COUNT/8 * CHAR_BIT, MSB_FIRST);
    if (msync(bitmap, BLOCK_COUNT/8, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo bitmap.dat");
        munmap(bitmap, BLOCK_COUNT/8);
        close(fd);
        return ;
    }
    //si el nuevo tamanio es menor al anterior, se liberan bloques
    if (tamanio<tamanoArchivo) {
        printf("tamanio<tamanoArchivo\n");
        // Calcular la cantidad de bloques ocupados por el archivo
        int bloquefinalprev = bloqueInicial+bloquesarchivo-1;
        int bloquefinalpost = bloqueInicial+cantidadBloques;
        printf("bloquefinalprev: %d\n", bloquefinalprev);//printf("bloquefinalpost: %d\n", bloquefinalpost);
        // Liberar los bloques ocupados por el archivo
        for (int i = bloquefinalprev; i >=bloquefinalpost; i--)
            bitarray_clean_bit(my_bitmap2, i);
        escribir_metadata(nombreSinExtension, bloqueInicial, tamanio);//modificar metadata
    }
    else if (tamanio>tamanoArchivo) {//si el nuevo tamanio es mayor al anterior, se buscan bloques libres y se los asigna
        //contar la cantidad de bloques libres depues de que termina el archivo
        printf("tamanio>tamanoArchivo\n");
        int bloqueslibres=0;
        for (int i = bloquefinal+1; !bitarray_test_bit(my_bitmap2, i); i++) 
            bloqueslibres++;
        //printf("bloqueslibres: %d\n", bloqueslibres); //cantidad de bloques libres que se encuentran despues del archivo que se quiere agrandar
        if(bloqueslibres>=cantidadBloques-1){
            printf("Se encontraron suficientes bloques libres para agrandar el archivo.\n");
            // Calcular la cantidad de bloques ocupados por el archivo
            int bloquefinalposta = bloqueInicial+cantidadBloques-1;//printf("bloquefinal: %d\n", bloquefinal);
            // Asignar los bloques libres al archivo
            for (int i = bloquefinal; i < bloquefinalposta; i++) 
                bitarray_set_bit(my_bitmap2, i+1);// posicionBit = i+1;printf("posicionBit: %d\n", posicionBit);
            escribir_metadata(nombreSinExtension, bloqueInicial, tamanio);//modificar metadata
        } else { 
            char* datos=leerDatosDesdeArchivo(nombre, 0, tamanoArchivo);
            printf("datos de archivo a agrandar: %s\n", datos);
            log_info(logger, "Inicio Compactación.");
            borrarContenidoArchivo(nombre, tamanoArchivo);
            DIR *dir;
            struct dirent *ent;
            int nuevaposicioninicial, nuevaposicionfinal;
            int iteracion=0;
            if ((dir = opendir(PATH_BASE_DIALFS)) != NULL) {
                // Lee cada entrada del directorio
                while ((ent = readdir(dir)) != NULL) {
                    // Verifica si el nombre del archivo termina con ".metadata"
                    size_t len = strlen(ent->d_name);
                    if (len >= 9 && strcmp(ent->d_name + len - 9, ".metadata") == 0 && strcmp(ent->d_name, pathMetadatabeta)!=0) {
                        char pathMetadata[324];//printf("%s\n", ent->d_name);
                        sprintf(pathMetadata, "%s/%s", PATH_BASE_DIALFS, ent->d_name);
                        char otropath[324];
                        sprintf(otropath, "%s/txt", ent->d_name);
                        t_config *config2 = config_create(pathMetadata);
                        if (config2 == NULL) {
                            perror("Error al abrir el archivo de metadata");
                            return ;
                        }
                        int bloqueInicialotroarchivo = config_get_int_value(config2, "bloqueInicial");
                        int tamanoArchivootroarchivo = config_get_int_value(config2, "tamanoArchivo");
                        //printf("bloqueInicial: %d\n", bloqueInicial);printf("tamanoArchivo: %d\n", tamanoArchivo)
                        int bloquesarchivo = tamanoArchivootroarchivo/BLOCK_SIZE;
                        if (tamanoArchivootroarchivo%BLOCK_SIZE!=0) 
                            bloquesarchivo++;
                        if (tamanoArchivootroarchivo==0)
                            bloquesarchivo = 1;
                        char valorBloqueInicial[20];
                        if (bloqueInicialotroarchivo>bloquefinal && iteracion==0) {//el archivo accedido se encuentra despues del que se quiere agrandar
                            char *datosDeOtroArchivo=leerDatosDesdeArchivo(ent->d_name, 0, tamanoArchivootroarchivo);
                            borrarContenidoArchivo(otropath,  tamanoArchivootroarchivo);
                            bloqueInicialotroarchivo=bloqueInicial;
                            sprintf(valorBloqueInicial, "%d", bloqueInicialotroarchivo);
                            config_set_value(config2, "bloqueInicial", valorBloqueInicial);
                            if (config_save(config2) == -1) 
                                perror("Error al guardar el archivo de metadata");
                            nuevaposicionfinal=bloqueInicialotroarchivo+bloquesarchivo-1;
                            nuevaposicioninicial=nuevaposicionfinal+1;
                            for (int i = bloqueInicial; i < nuevaposicionfinal; i++) 
                                bitarray_set_bit(my_bitmap2, i+1);// posicionBit = i+1;printf("posicionBit: %d\n", posicionBit);
                            escribirCadenaEnArchivo(ent->d_name, datosDeOtroArchivo, 0);
                            free(datosDeOtroArchivo);
                            iteracion++;
                        }
                        if (bloqueInicialotroarchivo>bloquefinal && iteracion>0) {//el archivo accedido se encuentra despues del que se quiere agrandar
                            bloqueInicialotroarchivo=nuevaposicioninicial;//borrarContenidoArchivo(ent->d_name, 0, tamanoArchivootroarchivo);
                            char *datosDeOtroArchivo=leerDatosDesdeArchivo(ent->d_name, 0, tamanoArchivootroarchivo);
                            sprintf(valorBloqueInicial, "%d", bloqueInicialotroarchivo);
                            config_set_value(config2, "bloqueInicial", valorBloqueInicial);
                            if (config_save(config2) == -1) 
                                perror("Error al guardar el archivo de metadata");
                            nuevaposicionfinal=bloqueInicialotroarchivo+bloquesarchivo-1;
                            nuevaposicioninicial=nuevaposicionfinal+1;
                            for (int i = bloqueInicial; i < nuevaposicionfinal; i++) 
                                bitarray_set_bit(my_bitmap2, i+1);// posicionBit = i+1;printf("posicionBit: %d\n", posicionBit);
                            escribirCadenaEnArchivo(ent->d_name, datosDeOtroArchivo, 0);
                            free(datosDeOtroArchivo);
                        }
                        config_destroy(config2);
                    }
                }
                closedir(dir); // Cierra el directorio después de leer
                log_info(logger, "Fin Compactación.");
            } else {
                perror("Error al abrir el directorio");
                return ;
            }
            usleep(RETRASO_COMPACTACION*5);
            for (int i = nuevaposicioninicial; i < nuevaposicioninicial+cantidadBloques-1; i++) 
                bitarray_set_bit(my_bitmap2, i+1);//posicionBit = i+1;printf("posicionBit: %d\n", posicionBit);
            char *sinextension=obtenerNombreSinExtension(pathMetadatabeta);
            printf("nombreSinExtension: %s\n", sinextension);
            escribir_metadata(sinextension, nuevaposicioninicial, tamanio);//modificar metadata del archivo truncado
            escribirCadenaEnArchivo(pathMetadatabeta, datos, 0);
            free(datos);
        }
    }
    if (msync(bitmap, BLOCK_COUNT/8, MS_SYNC) == -1) 
        perror("Error al sincronizar el archivo bitmap.dat");
    truncate(pathArchivo, tamanio);
    mostrar_tamano_archivo(pathArchivo);
    printf("FIN TRUNCAR ARCHIVO.....\n");
    // Liberar recursos
    munmap(bitmap, BLOCK_COUNT/8);
    fclose(file);
    bitarray_destroy(my_bitmap2);
    close(fd);
}

void delete_file(const char *nombre) {
    usleep(TIEMPO_UNIDAD_TRABAJO*1000);
    log_info(logger, "Borrar archivo: <%s>", nombre);
    char pathArchivo[256];
    sprintf(pathArchivo, "%s/%s", PATH_BASE_DIALFS, nombre);
    //NO hace falta borrar el contenido en bloques.dat
    if (remove(pathArchivo) != 0) {// Eliminar el archivo
        perror("Error al borrar el archivo");
        return;
    } 
    char pathBitmap[128];
    sprintf(pathBitmap, "%s/bitmap.dat", PATH_BASE_DIALFS);
    int fd = open(pathBitmap, O_RDWR);// Abrir el archivo bitmap.dat
    if (fd == -1) {
        perror("Error al abrir el archivo bitmap.dat");
        return;
    } 
    // Mapear el archivo bitmap.dat en memoria
    void *bitmap = mmap(NULL, BLOCK_COUNT/8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return;
    }
    // Crear el bitarray a partir del bitmap mapeado
    t_bitarray *my_bitmap2 = bitarray_create_with_mode(bitmap, BLOCK_COUNT/8 * CHAR_BIT, MSB_FIRST);
    // abrir su metadata como lectura para obtener bloque inicial y tamanio
    char pathMetadata[256];
    char *nombreSinExtension = obtenerNombreSinExtension(nombre);
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombreSinExtension);
    t_config *config = config_create(pathMetadata);
    if (config == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");//printf("bloqueInicial: %d\n", bloqueInicial);
    //tamanio del archivo en bytes
    int tamanoArchivo = config_get_int_value(config, "tamanoArchivo");
    // Calcular la cantidad de bloques ocupados por el archivo
    int bloquesOcupados= tamanoArchivo/BLOCK_SIZE;
    if (tamanoArchivo%BLOCK_SIZE!=0) 
        bloquesOcupados++;
    if (tamanoArchivo==0)
        bloquesOcupados=1;
    //printf("bloquesOcupados: %d\n", bloquesOcupados);
    int bloquefinal= bloqueInicial+bloquesOcupados;
    // Liberar los bloques ocupados por el archivo
    for (int i = bloquesOcupados; i >0; i--) 
        bitarray_clean_bit(my_bitmap2, bloquefinal-i);
    // Sincronizar los cambios en el archivo bitmap.dat
    if (msync(bitmap, BLOCK_COUNT/8, MS_SYNC) == -1) 
        perror("Error al sincronizar el archivo bitmap.dat");
    if (remove(pathMetadata) != 0) {
        perror("Error al borrar el archivo de metadata");
        return;
    }
    // Liberar recursos
    config_destroy(config);
    munmap(bitmap, BLOCK_COUNT/8);
    bitarray_destroy(my_bitmap2);
    close(fd);
}

void crearArchivo2(char* nombre) {
    usleep(TIEMPO_UNIDAD_TRABAJO*1000);
    log_info(logger, "Crear archivo: <%s>", nombre);
    char pathArchivo[256];
    sprintf(pathArchivo, "%s/%s", PATH_BASE_DIALFS, nombre);
    FILE *file = fopen(pathArchivo, "w"); // Abrir el archivo en modo escritura ('w')
    if (file == NULL) {
        perror("Error al crear el archivo");
        return;
    }
    char pathBitmap[256];
    sprintf(pathBitmap, "%s/bitmap.dat", PATH_BASE_DIALFS);
    int fd = open(pathBitmap, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo bitmap.dat");
        return ;
    } 
    void *bitmap = mmap(NULL, BLOCK_COUNT/8, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED) {
        perror("Error al mapear el archivo bitmap.dat");
        close(fd);
        return ;
    }
    t_bitarray *my_bitmap2 = bitarray_create_with_mode(bitmap, BLOCK_COUNT/8 * CHAR_BIT, MSB_FIRST);
    int primerBloqueLibre =obtenerPrimeraPosicionLibre(my_bitmap2);
    bitarray_set_bit(my_bitmap2, primerBloqueLibre);
    if (msync(bitmap, BLOCK_COUNT/8, MS_SYNC) == -1) {
        perror("Error al sincronizar el archivo bitmap.dat");
        munmap(bitmap, BLOCK_COUNT/8);
        close(fd);
        return ;
    }
    char* nombresinextension = obtenerNombreSinExtension(nombre);
    crearMetadata(nombresinextension, primerBloqueLibre);
    munmap(bitmap, BLOCK_COUNT/8);
    close(fd);
    bitarray_destroy(my_bitmap2);
}

void create_bitmap_file(const char *filename, size_t size) {
    char pathBitmap[256];
    sprintf(pathBitmap, "%s/%s", PATH_BASE_DIALFS, filename);
    int fd = open(pathBitmap, O_RDWR);
    if (fd == -1) {// El archivo no existe, lo creamos
        fd = open(pathBitmap, O_RDWR | O_CREAT, 0666);
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
    // Destruir el bitarray y liberar recursos
    bitarray_destroy(my_bitmap);
    munmap(bitmap, size);
    close(fd);
}

void escribir_metadata(char *nombre, int bloqueInicial, int tamanoArchivo) {
    // Construir el nombre del archivo de metadata
    char pathMetadata[128];
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombre);
    // Crear un archivo de configuración para el metadata
    t_config *config = config_create(pathMetadata);
    if (config == NULL) {
        perror("Error al crear el archivo de metadata");
        return;
    }
    // Convertir los valores a strings y guardarlos en el archivo de metadata
    char valorBloqueInicial[20], valorTamanoArchivo[20];
    sprintf(valorBloqueInicial, "%d", bloqueInicial);
    sprintf(valorTamanoArchivo, "%d", tamanoArchivo);
    config_set_value(config, "bloqueInicial", valorBloqueInicial);
    config_set_value(config, "tamanoArchivo", valorTamanoArchivo);
    // Guardar los cambios en el archivo
    if (config_save(config) == -1) 
        perror("Error al guardar el archivo de metadata");
    // Liberar recursos
    config_destroy(config);
}

void create_bloques_file(const char *filename, size_t size) {
    char pathBloques[256] = PATH_BASE_DIALFS;
    sprintf(pathBloques, "%s/%s", PATH_BASE_DIALFS, filename);
    int fd = open(pathBloques, O_RDWR);
    if (fd == -1) {
        // El archivo no existe, lo creamos
        fd = open(pathBloques, O_RDWR | O_CREAT, 0666);
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
}

void crearMetadata(char *nombre, int bloqueInicial){   
    char pathMetadata[128];
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombre);
    FILE *fileMetadata = fopen(pathMetadata, "w");
    if (fileMetadata == NULL) {
        perror("Error al crear el archivo de metadata");
        return;
    }
    fclose(fileMetadata);    
    // Crear un archivo de configuración para el metadata
    t_config *config = config_create(pathMetadata);
    if (config == NULL) {
        perror("Error al crear el archivo de metadata");
        return;
    }
    // Convertir los valores a strings y guardarlos en el archivo de metadata
    char valorBloqueInicial[20], valorTamanoArchivo[20];
    sprintf(valorBloqueInicial, "%d", bloqueInicial);
    sprintf(valorTamanoArchivo, "%d", 0);
    config_set_value(config, "bloqueInicial", valorBloqueInicial);
    config_set_value(config, "tamanoArchivo", valorTamanoArchivo);
    // Guardar los cambios en el archivo
    if (config_save(config) == -1) 
        perror("Error al guardar el archivo de metadata");
    config_destroy(config);
}

int obtenerPrimeraPosicionLibre(t_bitarray *bitmap) {
    int posicion;
    int size = bitmap->size;
    for (int i = 0; i < size; i++) 
        if (!bitarray_test_bit(bitmap, i)) {
            posicion = i;
            break;  // Salir del bucle al encontrar el primer bit libre
        }
    return posicion;
}

void mostrar_tamano_archivo(const char *nombre) {
    const char *pathArchivo = nombre;
    struct stat st;
    if (stat(pathArchivo, &st) == 0) {
        printf("El tamaño del archivo es: %lld bytes\n", (long long)st.st_size);
    } else {
        perror("stat");
        return ;
    }
}

char* obtenerNombreSinExtension(const char *nombreArchivo) {
    static char nombreSinExtension[256]; 
    strcpy(nombreSinExtension, nombreArchivo); // Copiar el nombre original al buffer
    char *punto = strrchr(nombreSinExtension, '.');
    if (punto != NULL) 
        *punto = '\0'; // Colocar el terminador nulo para eliminar la extensión
    return nombreSinExtension;
}

char* leerDatosDesdeArchivo(const char *nombreArchivo, off_t offset, size_t cantidadBytes) {
    char pathArchivo[256] = PATH_BASE_DIALFS;
    sprintf(pathArchivo, "%s/bloques.dat", PATH_BASE_DIALFS);
    // Abrir el archivo
    int fd = open(pathArchivo, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return NULL;
    }
    char pathMetadata[128];
    char *nombre = obtenerNombreSinExtension(nombreArchivo);
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombre);
    t_config *config = config_create(pathMetadata);
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    int posicionInicial = bloqueInicial*BLOCK_SIZE+offset;
    // Mapear el archivo en memoria
    void *map = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        return NULL;
    }
    // Verificar si el offset más la cantidad de bytes supera el tamaño del archivo mapeado
    if (posicionInicial + cantidadBytes > BLOCK_COUNT * BLOCK_SIZE) {
        fprintf(stderr, "Error: El offset y la cantidad de bytes exceden el tamaño del archivo.\n");
        munmap(map, BLOCK_COUNT * BLOCK_SIZE);
        close(fd);
        return NULL;
    }
    // Reservar memoria para almacenar los datos leídos
    char *datosLeidos = (char *)malloc(cantidadBytes + 1); // +1 para el terminador nulo
    if (datosLeidos == NULL) {
        perror("Error al reservar memoria para los datos leídos");
        munmap(map, BLOCK_COUNT * BLOCK_SIZE);
        close(fd);
        return NULL;
    }
    // Copiar los datos desde el archivo mapeado al buffer de datos leídos
    memcpy(datosLeidos, (char *)map + posicionInicial, cantidadBytes);
    datosLeidos[cantidadBytes] = '\0'; // Asegurar que la cadena esté terminada en nulo
    // Desmapear el archivo y cerrar el descriptor de archivo
    if (munmap(map, BLOCK_COUNT * BLOCK_SIZE) == -1) {
        perror("Error al desmapear el archivo");
    }
    close(fd);
    config_save(config);
    config_destroy(config);
    //free(datosLeidos);
    return datosLeidos;
}

void escribirCadenaEnArchivo(const char *nombreArchivo, const char *cadena, off_t offset) {
    char pathArchivo[256] = PATH_BASE_DIALFS;
    sprintf(pathArchivo, "%s/bloques.dat", PATH_BASE_DIALFS);
    size_t tamanioCadena = strlen(cadena) + 1; // Longitud de la cadena más el caracter nulo '\0'
    // Abrir el archivo
    int fd = open(pathArchivo, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return;
    }
    char pathMetadata[128];
    char *nombre = obtenerNombreSinExtension(nombreArchivo);
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombre);
    t_config *config = config_create(pathMetadata);
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    printf("bloqueInicial: %d\n", bloqueInicial);
    int posicionInicial = bloqueInicial*BLOCK_SIZE+offset;
    // Mapear el archivo en memoria
    void *map = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        return;
    }
    // Escribir la cadena en la memoria mapeada
    memcpy((char *)map + posicionInicial, cadena, tamanioCadena);
    // Sincronizar los cambios con el archivo en disco
    if (msync(map, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC) == -1) 
        perror("Error al sincronizar los cambios con el archivo");
    // Desmapear el archivo y cerrar el descriptor de archivo
    if (munmap(map, BLOCK_COUNT * BLOCK_SIZE) == -1) 
        perror("Error al desmapear el archivo");
   // config_save(config);
    config_destroy(config);
    close(fd);
}

void escribirIntEnArchivo(const char *nombreArchivo, int dato, off_t offset) {
    char pathArchivo[256] = PATH_BASE_DIALFS;
    sprintf(pathArchivo, "%s/bloques.dat", PATH_BASE_DIALFS);
    size_t tamanioInt = sizeof(int); // Tamaño del entero
    // Abrir el archivo
    int fd = open(pathArchivo, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return;
    }
    char pathMetadata[128];
    char *nombre = obtenerNombreSinExtension(nombreArchivo);
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombre);
    t_config *config = config_create(pathMetadata);
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    int posicionInicial = bloqueInicial*BLOCK_SIZE+offset;
    // Mapear el archivo en memoria
    void *map = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        close(fd);
        return;
    }
    // Escribir el entero en la memoria mapeada
    memcpy((char *)map + posicionInicial, &dato, tamanioInt);
    // Sincronizar los cambios con el archivo en disco
    if (msync(map, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC) == -1)
        perror("Error al sincronizar los cambios con el archivo");
    // Desmapear el archivo y cerrar el descriptor de archivo
    if (munmap(map, BLOCK_COUNT * BLOCK_SIZE) == -1) 
        perror("Error al desmapear el archivo");
    close(fd);
}

void borrarContenidoArchivo(const char *nombreArchivo, size_t cantidadBytes) {
    char pathArchivo[256];
    sprintf(pathArchivo, "%s/bloques.dat", PATH_BASE_DIALFS);
    // Abrir el archivo
    int fd = open(pathArchivo, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return;
    }
    char pathMetadata[128];
    char *nombreSinExtension = obtenerNombreSinExtension(nombreArchivo);
    sprintf(pathMetadata, "%s/%s.metadata", PATH_BASE_DIALFS, nombreSinExtension);
    t_config *config = config_create(pathMetadata);
    if (config == NULL) {
        perror("Error al abrir el archivo de metadata");
        close(fd);
        return;
    }
    int bloqueInicial = config_get_int_value(config, "bloqueInicial");
    if (bloqueInicial < 0) {
        fprintf(stderr, "Valor de bloqueInicial inválido\n");
        config_destroy(config);
        close(fd);
        return;
    }
    // Mapear el archivo en memoria
    void *map = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error al mapear el archivo");
        config_destroy(config);
        close(fd);
        return;
    }
    int posicionInicial = BLOCK_SIZE * bloqueInicial;
    if (posicionInicial + cantidadBytes > BLOCK_COUNT * BLOCK_SIZE) {
        fprintf(stderr, "Rango de bytes a borrar excede el tamaño del archivo\n");
        munmap(map, BLOCK_COUNT * BLOCK_SIZE);
        config_destroy(config);
        close(fd);
        return;
    }
    // Escribir ceros en la memoria mapeada
    memset((char *)map + posicionInicial, 0, cantidadBytes);
    // Sincronizar los cambios con el archivo en disco
    if (msync(map, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC) == -1) 
        perror("Error al sincronizar los cambios con el archivo");
    // Desmapear el archivo y cerrar el descriptor de archivo
    if (munmap(map, BLOCK_COUNT * BLOCK_SIZE) == -1) 
        perror("Error al desmapear el archivo");
   // config_save(config);
    config_destroy(config);
    close(fd);
}