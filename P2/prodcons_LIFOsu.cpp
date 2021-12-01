/*
   SCD - Práctica 2 - Casos prácticos de monitores
   Problema del productor-consumidor con buffer LIFO.
   Luis Miguel Guirado Bautista
   2021/2022
*/

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_items  = 20,    // número de items a producir/consumir
   tam_buf = 5, // tamaño del buffer
   num_prod = 4, // numero de productores
   num_cons = 4, // numero de consumidores
   val_x_prod = num_items/num_prod; // valores por productor

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items], // contadores de verificación: consumidos
   items_prod[num_prod] = {0}; // indica cuantos valores han producido las hebras productoras

/////////////////////////////////////////////////////////////////////////////////////////////
// procedimientos externos

void espera() { this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ) ); }

int producir_dato( unsigned int num_hebra )
{
   espera();

   const int val_producido = (val_x_prod*num_hebra)+items_prod[num_hebra];

   mtx.lock();
   cout << "producido: " << val_producido << endl << flush ;
   mtx.unlock();

   cont_prod[val_producido] ++ ;
   items_prod[num_hebra]++;

   return val_producido ;
}

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato == " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }

   cont_cons[dato] ++ ;

   espera();

   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// declaracion del monitor SU (buffer LIFO)
class ProdConsSU : public HoareMonitor {
    private:
        int buffer[num_items] = {0};
        int primera_libre, primera_ocupada;
        CondVar libres;
        CondVar ocupadas;
    public:
        ProdConsSU();
        void insertar( int dato );
        int extraer();
};
/////////////////////////////////////////////////////////////////////////////////////////////
// inicializacion e implementacion de procedimientos
ProdConsSU::ProdConsSU() {

    primera_libre = primera_ocupada = 0;
    libres = newCondVar();
    ocupadas = newCondVar();
}

void ProdConsSU::insertar( int dato ) {

    if ( primera_libre == tam_buf )
        libres.wait();

    assert( primera_libre < tam_buf );

    buffer[primera_libre] = dato;
    primera_libre = (primera_libre + 1) % tam_buf;

    ocupadas.signal();
}

int ProdConsSU::extraer() {

    int dato;
    if ( primera_libre == primera_ocupada )
        ocupadas.wait();

    dato = buffer[primera_ocupada];
    primera_ocupada = (primera_ocupada + 1) % tam_buf;

    libres.signal();
    return dato;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// funciones de hebras
void funcion_hebra_productora(MRef<ProdConsSU> monitor, unsigned int num_hebra )
{
    for (unsigned i = 0; i < val_x_prod; i++)
    {
        int valor = producir_dato( num_hebra );
        monitor->insertar(valor);
    }
}

void funcion_hebra_consumidora(MRef<ProdConsSU> monitor)
{
    for (unsigned i = 0; i < val_x_prod; i++)
    {
        int valor = monitor->extraer();
        consumir_dato(valor);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
   // num_prod y num_cons deben ser divisores de num_items, si no, puede haber errores de ejecucion
   if (num_items % num_prod != 0 or num_cons % num_cons != 0) {
      cerr << "ERROR: num_prod o num_cons no es divisor de num_items" << endl;
      return 1;
   }

    cout << "-------------------------------------------------------------------------------" << endl
         << "Problema de los productores-consumidores (" << num_prod << " prod/" << num_cons << " cons, Monitor SU, buffer LIFO). " << endl
         << "-------------------------------------------------------------------------------" << endl
         << flush;

   ini_contadores();

    MRef<ProdConsSU> monitor = Create<ProdConsSU>();

    // Inicializamos las hebras
    thread productores[num_prod];
    for (int i=0; i < num_prod; i++)
        productores[i] = thread( funcion_hebra_productora, monitor, i );

    thread consumidores[num_cons];
    for (int i=0; i < num_cons; i++)
        consumidores[i] = thread( funcion_hebra_consumidora, monitor );

    // Esperamos a que terminen
    for (int i=0; i < num_prod; i++)
        productores[i].join();

    for (int i=0; i < num_cons; i++)
        consumidores[i].join();

    test_contadores();
}
