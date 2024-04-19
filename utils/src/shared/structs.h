#ifndef STRUCTS_H_
#define STRUCTS_H_


//Algoritmos de planificacion:

typedef enum {
     VRR,
     FIFO,
     UNKNOWN
} tAlgoritmosPlanificacion;

//tipos de interfaces para IO
 typedef enum {
     STDIN,
     STDOUT,
      DIALFS,
     GENERICA
} tInterfaces;
#endif /* STRUCTS_H_ */