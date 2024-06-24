#ifndef UTILS_CLIENTE_H
#define UTILS_CLIENTE_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <configuraciones/configuraciones.h>

#define SEGS_ANTES_DE_REINTENTO 3

extern t_log *logger;
extern t_config *config;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef enum
{	
	MENSAJE,
	PAQUETE, 
	CONTEXTOEJECUCION,
	TABLADEPAGINAS,
	READ, 
	WRITE,
	NEWPCB, 
    ENDPCB,
	CREATE_PAGE_OP,
	DELETE_PAGE_OP,
	TERMINAR_KERNEL,
	SUCCESS,
	OUTOFMEMORY,
	COMPACTACION,
	FOPEN,
	FCREATE,
	FTRUNCATE,
	FREAD,
	FWRITE,
	FDEL
} op_code;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;

} t_paquete;
typedef struct
{
	op_code codigo_operacion;
    t_buffer* buffer;
    int entero;

} t_paquetebeta;
/**
 * @fn int crearConexion(char* ip, char* puerto)
 * @brief Dados una direccion IP y un puerto, se conecta a dicha IP en dicho puerto y genera el socket de conexión para interactuar con este servidor.
 * @param ip Dirección a la que conectarse.
 * @param puerto Puerto al que solicitar un socket disponible.
 * @return Socket con la conexión realizada.
 */
int crearConexion(char* ip, char* puerto);
/**
 * @fn void enviarMensaje(char* mensaje, int socketCliente)
 * @brief Se envia un paquete del tipo MENSAJE que envia un string al socket solicitado.
 * @param mensaje El string a enviar.
 * @param socketCliente El socket mediante el cual enviar el mensaje.
 */
void enviarMensaje(char* mensaje, int socketCliente);
/**
 * @fn t_paquete* crearPaquete(void)
 * @brief Se inicializa un paquete con código de operación PAQUETE.
 * @return Retorna el paquete antes inicializado, o -1 en caso de error.
 */
t_paquete* crearPaquete(void);
/**
 * @fn void agregarAPaquete(t_paquete* paquete, void* valor, int tamanio)
 * @brief Se agrega un void* al paquete elegido, apendeandolo al final del paquete.
 * @param paquete Paquete al que se agrega el valor.
 * @param valor Dato a añadir al paquete.
 * @param tamanio Tamaño del valor a añadir, usualmente un sizeof(valor) servira, pero en el caso de strings se puede utilizar (strlen(valor) + 1) * sizeof(int)
 */
void agregarAPaquete(t_paquete* paquete, void* valor, int tamanio);
/**
 * @fn void enviarPaquete(t_paquete* paquete, int socketCliente)
 * @brief Se envia un paquete mediante el socket utilizado.
 * @param paquete Paquete a enviar.
 * @param socketCliente Socket utilizado.
 */
void enviarPaquete(t_paquete* paquete, int socketCliente);
/**
 * @fn void eliminarPaquete(t_paquete* paquete)
 * @brief Elimina un paquete y libera la memoria asociada a sus datos.
 * @param paquete El puntero al paquete a eliminar.
 */
void eliminarPaquete(t_paquete* paquete);

void enviarCodOp (op_code codigoDeOperacion, int socket);

/**
 * @fn int conexion(char *SERVIDOR)
 * 
 * @brief Se realiza la conexion y se devuelve el socket generado, o un -1 en caso de fallar.
 * @param SERVIDOR Se envia el nombre del servidor al que se debe conectar para agarrar la informacion correcta de la configuracion y hacer la conexion.
 * @return int Se retorna el socket generado o un -1 en caso de fallar.
 */
int conexion(char *SERVIDOR);

/* Funciones Privadas */

/**
 * @brief Se aloca el buffer del paquete recibido.
 * @param paquete Paquete que aun no tiene el buffer inicializado.
 */
void crearBuffer(t_paquete *paquete);

/**
 * @brief Dado un paquete y la cantidad de bytes que contiene, se empaqueta la cantidad de bytes en un void,
 *        serializandolo y preparandolo para enviar a un servidor con el formato:
 *        1. codigo de operación del paquete
 *        2. tamaño del paquete
 *        3. el contenido del paquete en sí
 * @param paquete El paquete a serializar.
 * @param bytes La cantidad de bytes a serializar (usualmente sizeof(paquete)).
 * @return Se retorna el formato serializado como un void*.
 */
void *serializarPaquete(t_paquete *paquete, int bytes);

#endif /* UTILS_H_ */