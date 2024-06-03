#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

#include <global.h>
#include <configuraciones/configuraciones.h>
#include <planificacion/planificacion.h>
#include <peticiones/pcb.h>
#include <conexiones/conexionMemoria.h>

int ejecutarConsola (int, char *archivos[]);
void ejecutarScript(const char*);
void iniciarProceso(const char*);
void finalizarProceso(int);
void detenerPlanificacion();
void iniciarPlanificacion();
void procesoEstado();

extern t_log* logger;

#endif /* CLIENT_H_ */
