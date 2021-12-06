// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-cam.cpp
// Implementación del problema de los filósofos (con camarero).
//
// Versión de Luis Miguel Guirado Bautista. Curso 2021/22.
//
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_filo_ten  = 2*num_filosofos,
   num_procesos  = num_filo_ten + 1,   // + el camarero
   // Los filosofos realizan dos tipos de llamadas al camarero,
   // entonces, vamos a declarar dos etiquetas:
   etiq_sentarse = 1,
   etiq_levantarse  = 2;


//**********************************************************************

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten, //id. tenedor der.
      valor;
   MPI_Status estado;

  while ( true )
  {
    // 1. SENTARSE
    // Solicita un sitio para sentarse
    cout << "Filosofo " << id << " solicita sentarse" << endl;
    MPI_Ssend( &valor, 1, MPI_INT, num_filo_ten, etiq_sentarse, MPI_COMM_WORLD);

    // 2. SOLICITAR TENEDORES
    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    // 3. COMER
    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    // 4. SOLTAR TENEDORES
    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    MPI_Ssend( &valor, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    // 5. LEVANTARSE
    // Solicita levantarse
    cout << "Filosofo " << id << " quiere levantarse" << endl;
    MPI_Ssend( &valor, 1, MPI_INT, num_filo_ten, etiq_levantarse, MPI_COMM_WORLD);

    // 6. PENSAR
    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos

  while ( true )
  {
     // Recibimos la solicitud del filósofo para ocuparse y guardamos su identificador
     MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);
     id_filosofo = estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // Recibimos la solicitud del filósofo para liberarse
     MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero( int id ) {

   int valor, sentados = 0;
   MPI_Status estado;

   while (true) {
      if (sentados < 4) {
         // Recibimos la llamada para sentarse de un filósofo e incrementamos el contador 'sentados'
         MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_sentarse, MPI_COMM_WORLD, &estado );
         cout << "Camarero le concede el sitio al filosofo " << estado.MPI_SOURCE << endl;
         sentados++;
      }
      // Recibimos la llamada para levantarse de un filósofo y decrementamos el contador 'sentados'
      //? Este if no es necesario, pero podemos usarlo para tener un mínimo
      //? de filósofos sentados durante toda la ejecución
      if (sentados >= 0) {
         MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_levantarse, MPI_COMM_WORLD, &estado );
         cout << "Camarero le dice al filosofo " << estado.MPI_SOURCE << " que puede irse" << endl;
         sentados--;
      }
   }

}

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio < num_filo_ten ) {
         if ( id_propio % 2 == 0 )          // si es par
            funcion_filosofos( id_propio ); //   es un filósofo
         else                               // si es impar
            funcion_tenedores( id_propio ); //   es un tenedor
      }
      else funcion_camarero( id_propio ); // el ultimo (10) es camarero
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
