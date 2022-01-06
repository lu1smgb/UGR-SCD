// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
// Implementación de la actividad 2:
//
//   Datos de las tareas:
//   -------------
//   Ta.  T     C
//   -------------
//   A  500   100
//   B  500   150
//   C  1000  200
//   D  2000  240
//   -------------
//
//  Planificación (con Ts == 250 ms)
//  *-----------------------------------------*
//  | A B | C | A B | D | A B | C | A B | A B |
//  *-----------------------------------------*
//
//
// Historial:
// Creado en Diciembre de 2017
// -----------------------------------------------------------------------------

/*
   PREGUNTAS:

   - ¿ cual es el mínimo tiempo de espera que queda al final de las
      iteraciones del ciclo secundario con tu solución ?

   El tiempo de respuesta mínimo entre cada iteración del bucle secundario en mi
   equipo es de 0.343652 ms, 0.273674 ms, 0.489622 ms...
   De modo que al cabo de un rato el mínimo tiempo de respuesta siempre bajará
   del medio milisegundo

   - ¿ sería planificable si la tarea D tuviese un tiempo cómputo de
      250 ms ?

   Si siempre que D ocupe un tiempo de cómputo mayor o igual al tiempo de ejecución del
   ciclo secundario (Ts), como es en este caso
*/

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
//typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds(150) );  }
void TareaC() { Tarea( "C", milliseconds(200) );  }
void TareaD() { Tarea( "D", milliseconds(250) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms( 250 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   // Para registrar el tiempo de respuesta minimo entre todas las iteraciones del
   // ciclo principal
   float min_tr = 100000;

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB();                     break ;
            case 2 : TareaC();                               break ;
            case 3 : TareaD();                               break ;
            case 4 : TareaA(); TareaB();                     break ;
            case 5 : TareaC();                               break ;
            case 6 : TareaA(); TareaB();                     break ;
            case 7 : TareaC();                               break ;
            case 8 : TareaA(); TareaB();                     break ;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );

      }

		// Informamos del retraso final actual vs esperado
		time_point<steady_clock> fin_sec = steady_clock::now();
		float retraso = milliseconds_f( fin_sec - ini_sec ).count();
		cout << "\n*** Ciclo secundario terminado con retraso de " << retraso << " ms ***\n";

      if (retraso < min_tr) min_tr = retraso;
      cout << "*** Minimo tiempo de respuesta registrado: " << min_tr << " ms ***\n";
   }
}
