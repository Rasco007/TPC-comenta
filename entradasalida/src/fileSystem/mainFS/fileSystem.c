#include "fileSystem/mainFS/fileSystem.h"

int socketCliente, socketMemoria, fdBitmap, fdBloques;
int cantBloques, tamanioBitmap, tamanioBloques;
//t_log * logger, * loggerError; 
//t_config * config, * superbloque;
t_config* superbloque;
t_bitarray * bitmap;
char * ptrBloques, * ptrBitMap, ** bloques;
char * pathSuperBloque, * pathBloques, * pathBitmap, * pathFCBs;

int fileSystem() {
    logger = iniciarLogger("fileSys.log","File System");
    config = iniciarConfiguracion("filesys.config");
    loggerError = iniciarLogger("errores.log", "File System"); 
    
    atexit(terminarPrograma);

    conexionMemoria();
    atexit(cerrarConexion);


    pathSuperBloque = confGet("PATH_SUPERBLOQUE");
    pathBitmap = confGet("PATH_BITMAP");
    pathBloques = confGet("PATH_BLOQUES");
    pathFCBs = confGet("PATH_FCB");

    superbloque = config_create(pathSuperBloque);
    cantBloques = config_get_int_value(superbloque, "BLOCK_COUNT");
    tamanioBitmap = BIT_CHAR(cantBloques);
    tamanioBloques = config_get_int_value(superbloque, "BLOCK_SIZE");

    atexit(cerrarSuperBloque);

    fdBitmap = open(pathBitmap, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fdBitmap < 0)
        error("No se abrio correctamente el archivo %s; error: %s", pathBitmap, strerror(errno));
    atexit(cerrarArchivoBitmap);
    
    if (ftruncate(fdBitmap, tamanioBitmap) < 0)
        error("No se pudo expandir correctamente el archivo %s; error: %s", pathBitmap, strerror(errno));

    ptrBitMap = mmap(0, tamanioBitmap, PROT_WRITE | PROT_READ, MAP_SHARED, fdBitmap, 0);
    if (ptrBitMap == MAP_FAILED) 
        error("No se mapeo correctamente el bitmap; error: %s", strerror(errno));
    atexit(cerrarMMapBitmap);
    bitmap = bitarray_create_with_mode(ptrBitMap, cantBloques, MSB_FIRST);
    atexit(cerrarBitmap);

    fdBloques = open(pathBloques, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fdBloques < 0)
        error("No se abrio correctamente el archivo %s; error: %s", pathBloques, strerror(errno));
    atexit(cerrarArchivoBloques);

    if (ftruncate(fdBloques, cantBloques * tamanioBloques) < 0)
        error("No se pudo expandir correctamente el archivo %s; error: %s", pathBloques, strerror(errno));

    ptrBloques = mmap(0, tamanioBloques * cantBloques, PROT_WRITE | PROT_READ, MAP_SHARED, fdBloques, 0);
    if (ptrBloques == MAP_FAILED) 
        error("No se mapeo correctamente el bitmap; error: %s", strerror(errno));
    atexit(cerrarMMapBloques);
    
    bloques = malloc(cantBloques * sizeof (char *));
    atexit (limpiarBloques);

    for (int i = 0; i < cantBloques; i++) {
        *(bloques + i) = &ptrBloques[i * tamanioBloques];
    }

    if (mkdir (pathFCBs, S_IRUSR | S_IWUSR | S_IXUSR) == -1 && errno != EEXIST)
        error ("No se pudo crear o verificar que exista el directorio de FCBs, error: %s", strerror (errno));
    

    io_atender_kernel();
    exit(0);
}

void cerrarConexion () { close (socketMemoria); }
void cerrarSuperBloque () { config_destroy (superbloque); }
void cerrarBitmap () { bitarray_destroy(bitmap); }
void cerrarMMapBitmap () { munmap(ptrBitMap, tamanioBitmap); }
void cerrarArchivoBitmap () { close (fdBitmap); }
void limpiarBloques () { free (bloques); }
void cerrarMMapBloques () { munmap(ptrBloques, tamanioBloques); }
void cerrarArchivoBloques () { close (fdBloques); }