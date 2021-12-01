/*
   SCD - Práctica 2 - Casos prácticos de monitores
   Problema de los fumadores
   Luis Miguel Guirado Bautista
   2021/2022
*/

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <vector>
#include "scd.h"

using namespace std;
using namespace scd;

/////////////////////////////////////////////////////////////////////////////////////////////
// declaracion de variables globales
const int NUM_FUMADORES = 3;
/////////////////////////////////////////////////////////////////////////////////////////////
// declaración del monitor SU
class Estanco : public HoareMonitor {

    private:
        bool ocupado; // true si hay algun ingrediente en el mostrador
        short int disponibles[NUM_FUMADORES]; // Indica que ingredientes están disponibles
        CondVar cola_fumadores[NUM_FUMADORES]; // Colas para controlar a los fumadores
        CondVar cola_estanquero; // Cola para controlar la produccion del estanquero

    public:
        Estanco();
        void obtenerIngrediente( const unsigned int num_ing );
        void ponerIngrediente( const unsigned int num_ing );
        void esperarRecogidaIngrediente();
};
/////////////////////////////////////////////////////////////////////////////////////////////
// inicializacion e implementacion de los procedimientos
Estanco::Estanco() {

    ocupado = false;
    cola_estanquero = newCondVar();

    for (int i=0; i < NUM_FUMADORES; i++) {
        disponibles[i] = 0;
        cola_fumadores[i] = newCondVar();
    }
}

void Estanco::obtenerIngrediente( const unsigned int num_ing ) {

    cout << "Fumador " << num_ing << " espera su ingrediente" << endl;

    //? Cuando debe esperar el fumador
    if ( disponibles[num_ing] == 0 and not ocupado )
        cola_fumadores[num_ing].wait();

    disponibles[num_ing]--;
    ocupado = false;

    cout << "Fumador " << num_ing << " obtiene ingrediente" << endl;

    // El mostrador ya esta vacio, el estanquero puede seguir produciendo
    cola_estanquero.signal();
}

void Estanco::ponerIngrediente( const unsigned int num_ing ) {

    disponibles[num_ing]++;
    ocupado = true;

    cout << "Estanquero pone el ingrediente num. " << num_ing << " en el mostrador" << endl;

    // Indicamos al fumador num_ing que puede recoger su ingrediente
    cola_fumadores[num_ing].signal();
}

void Estanco::esperarRecogidaIngrediente() {

    cout << "Estanquero espera a que el mostrador este disponible" << endl;

    if ( ocupado )
        cola_estanquero.wait();

    cout << "Mostrador libre" << endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////

// para mayor legibilidad
void esperar() { this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ) ); }

/////////////////////////////////////////////////////////////////////////////////////////////
// procedimientos externos
void fumar( const int num_ing ) {

    cout << "Fumador " << num_ing << " empieza a fumar" << endl;

    esperar();

    cout << "Fumador " << num_ing << " termina de fumar" << endl;
}

int producirIngrediente() {

    cout << "Estanquero empieza a producir un ingrediente" << endl;

    int ingrediente = aleatorio<0, NUM_FUMADORES-1>();
    esperar();

    cout << "Estanquero produce el ingrediente " << ingrediente << endl;

    return ingrediente;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// procedimientos de hebras
void funcion_hebra_fumadora( MRef<Estanco> estanco, const int num_ing ) {

    while ( true ) {
        estanco->obtenerIngrediente( num_ing );
        fumar( num_ing );
    }
}

void funcion_hebra_estanquero( MRef<Estanco> estanco ) {
    
    while ( true ) {
        int ing = producirIngrediente();
        estanco->ponerIngrediente( ing );
        estanco->esperarRecogidaIngrediente();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
int main() {

    cout << "\tProblema de los fumadores (version monitor SU)\t" << endl;

    // Declaración del estanco
    MRef<Estanco> estanco = Create<Estanco>();

    // Declaración de los fumadores y del estanquero
    thread estanquero, fumadores[NUM_FUMADORES];
    estanquero = thread( funcion_hebra_estanquero, estanco );
    for ( int i=0; i < NUM_FUMADORES; i++ ) {
        fumadores[i] = thread( funcion_hebra_fumadora, estanco, i );
    }
    
    // Puesta en marcha
    estanquero.join();
    for (int i=0; i < NUM_FUMADORES; i++) {
        fumadores[i].join();
    }
    
    return 0;
}