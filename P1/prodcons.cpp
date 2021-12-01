/*
   SCD - Práctica 1 - Sincronización de hebras con semáforos
   Ejercicio del productor-consumidor (soluciones LIFO y FIFO)
   Luis Miguel Guirado Bautista
   2021/2022
*/

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <vector>
#include "scd.h"

//!! Cambie a 0 si desea comprobar la solución LIFO !!
#define FIFO 1

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 100 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

unsigned primera_libre = 0; // Indice de la primera celda libre
unsigned primera_ocupada = 0; // Indice de la primera celda ocupada
unsigned buffer[tam_vec] = {0}; // El buffer necesario para la realizacion del ejercicio

Semaphore puede_leer = 0, puede_escribir = tam_vec; // Semaforos necesarios para la concurrencia

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      //********************************************************
      sem_wait(puede_escribir); //? -1 espacio libre para producir
      
      buffer[primera_libre] = dato;
      if (FIFO <= 0)
         primera_libre++;
      else
         primera_libre = (primera_libre + 1) % tam_vec;

      sem_signal(puede_leer); //? +1 espacio a leer
      //********************************************************
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      //********************************************************
      sem_wait(puede_leer); //? -1 espacio a leer
      
      if (FIFO <= 0) {
         primera_libre--;
         dato = buffer[primera_libre];
      }
      else {
         dato = buffer[primera_ocupada];
         primera_ocupada = (primera_ocupada + 1) % tam_vec;
      }

      sem_signal(puede_escribir); //? +1 espacio libre para producir
      //********************************************************
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución " << (FIFO <= 0 ? "LIFO" : "FIFO") << ")" << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();
}
