// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons2-mu.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// en orden arbitrario
// (versión con múltiples productores y consumidores)
//
// Versión de Luis Miguel Guirado Bautista. Curso 2021/22.
//
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
    num_productores                   = 4 ,
    num_consumidores                  = 5 ,
    id_productores[num_productores]   = {0,1,2,3} ,
    id_buffer                         = num_productores,
    id_consumidores[num_consumidores] = {5,6,7,8,9} ,
    num_procesos_esperado             = 10 ,
    num_items                         = num_consumidores * num_productores * 2 ,
    items_por_productor               = num_items / num_productores ,
    items_por_consumidor              = num_items / num_consumidores ,
    etiq_buffer                       = 1,
    etiq_prod                         = 2,
    etiq_cons                         = 3,
    tam_vector                        = 10 ;


//**********************************************************************

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

int producir( int & orden )
{
   static int contador = orden * items_por_productor ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++;
   cout << "Productor ha producido valor " << contador << endl << flush;
   return contador ;
}

void funcion_productor( int & orden )
{
   for ( unsigned int i= 0 ; i < items_por_productor; i++ )
   {
      // producir valor
      int valor_prod = producir( orden );
      // enviar valor
      cout << "Productor va a enviar valor " << valor_prod << endl << flush;

      // Lo enviamos al buffer
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_prod, MPI_COMM_WORLD );
   }
}

void consumir( int valor_cons )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor ha consumido valor " << valor_cons << endl << flush ;
}

void funcion_consumidor( int & orden )
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < items_por_consumidor; i++ )
   {
      // Solicita permiso al buffer para poder consumir
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_cons, MPI_COMM_WORLD);

      // Permiso concedido: valor retirado del buffer
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_buffer, MPI_COMM_WORLD, &estado );
      cout << "Consumidor ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec );
   }
}

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiqueta;
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // Ahora la operación del buffer vendrá determinada por la etiqueta
      if ( num_celdas_ocupadas == 0 ) { // Cualquier productor
         etiqueta = etiq_prod;
      }
      else if ( num_celdas_ocupadas == tam_vector ) { // Cualquier consumidor
         etiqueta = etiq_cons;
      }
      else { // Cualquier proceso
         etiqueta = MPI_ANY_TAG ;
      }

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiqueta, MPI_COMM_WORLD, &estado );

      // Computar producción
      if ( estado.MPI_TAG == etiq_prod ) {

         buffer[primera_libre] = valor ;
         primera_libre = (primera_libre+1) % tam_vector ;
         num_celdas_ocupadas++ ;
         cout << "Buffer ha recibido valor " << valor << endl ;
      }
      // Computar consumo
      else if ( estado.MPI_TAG == etiq_cons ) {

         valor = buffer[primera_ocupada] ;
         primera_ocupada = (primera_ocupada+1) % tam_vector ;
         num_celdas_ocupadas-- ;
         cout << "Buffer va a enviar valor " << valor << endl ;
         MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, 1, MPI_COMM_WORLD);
      }
   }
}

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      if ( id_propio < num_productores )
         funcion_productor( id_propio );
      else if ( id_propio == id_buffer )
         funcion_buffer();
      else
         funcion_consumidor( id_propio );
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}