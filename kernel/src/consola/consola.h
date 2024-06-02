#ifndef CLIENT_H_
#define CLIENT_H_

/** 
 * @brief La consola tiene el trabajo de mandar al Kernel las instrucciones recibidas de los archivos.
 * 
 * La consola se divide en tres etapas principales:
 * 
 * - Inicio: Se prepara el programa abriendo la configuracion, el logger y el archivo de pseudocodigo, y se abre conexion con el Kernel, en caso de fallar cualquiera de estos puntos termina el programa con codigo de error = 1.
 * - Proceso: Se lee el archivo de pseudocodigo y se envia, linea por linea (y eliminando el corte de linea), al Kernel, en caso de falla tambien termina el programa.
 * - Final: Cierra los archivos y termina el programa con codigo de error 0.
 * 
 * A su vez, el proceso se divide en 4 partes:
 * 
 * + Se crea la linea de codigo.
 * + Se lee la linea y se eliminan los cortes de linea.
 * + Se crea el paquete, se prepara, y se envia.
 * + Se libera la memoria.
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include <global.h>
#include <configuraciones/configuraciones.h>
#include <planificacion/planificacion.h>
#include <peticiones/pcb.h>

int ejecutarConsola (int, char *archivos[]);
extern t_log* logger;

#endif /* CLIENT_H_ */
