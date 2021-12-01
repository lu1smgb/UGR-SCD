/*
   SCD - Práctica 1 - Sincronización de hebras con semáforos
   Ejercicio de los fumadores
   Luis Miguel Guirado Bautista
   2021/2022
*/

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;

// Declaración de semaforos
Semaphore mostrador_vacio = 1,
          ingrediente_disponible[num_fumadores] = {0,0,0};

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true) {
      int i = producir_ingrediente();
      sem_wait(mostrador_vacio); // El estanquero va a ocupar el mostrador ...
      cout << "Estanquero ha puesto el ingrediente " << i << " en el mostrador" << endl;
      sem_signal(ingrediente_disponible[i]); // ... con un ingrediente
   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      sem_wait(ingrediente_disponible[num_fumador]); // Va a dejar de estar disponible
      cout << "Fumador " << num_fumador << " ha recogido su ingrediente" << endl;
      sem_signal(mostrador_vacio); // Pero el mostrador vuelve a estar vacio
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   cout << "\t*** Problema de los fumadores ***\n";
   
   thread estanquero(funcion_hebra_estanquero),
          fumadores[num_fumadores];
   for (int i=0; i < num_fumadores; i++)
      fumadores[i] = thread(funcion_hebra_fumador,i);

   estanquero.join();
   for (int i=0; i < num_fumadores; i++) {
      fumadores[i].join();
   }
}
