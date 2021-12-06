// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 3. Introducción al paso de mensajes con MPI
//
// Archivo: holamundo.cpp
// Ejecución de varios procesos, cada uno de los cuales imprime un mensaje
// en la salida estándar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------

#include <mpi.h>
#include <iostream>
using namespace std;

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual ;

   // Ejecutamos el entorno MPI
   MPI_Init( &argc, &argv );

   // Guarda el número de procesos global en num_procesos actual
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   // Guarda el identificador de proceso dentro de MPI_COMM_WORLD en la variable id_propio
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );

   cout << "Hola desde proceso " << id_propio << " de " << num_procesos_actual << endl ;

   // Finaliza el entorno
   MPI_Finalize();
   return 0;
}

/*
   Para compilar y ejecutar:
   
   mpicxx -std=c++11 -o hola.out holamundo.cpp
   mpirun -np 4 ./hola.out
*/
