/*
   SCD - Práctica 2 - Casos prácticos de monitores
   Problema de los lectores-escritores
   Luis Miguel Guirado Bautista
   2021/2022
*/

#include <iostream>
#include <assert.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "scd.h"

using namespace std;
using namespace scd;

/////////////////////////////////////////////////////////////////////////////////////////////
// declaración del monitor SU
class Lec_Esc : public HoareMonitor {

    private:
        bool escrib; // true si esta escribiendo
        int n_lec; // numero de lectores leyendo
        CondVar lectura;
        CondVar escritura;
    public:
        Lec_Esc();
        void ini_lectura();
        void ini_escritura();
        void fin_lectura();
        void fin_escritura();
};
/////////////////////////////////////////////////////////////////////////////////////////////
// inicializacion e implementacion de los metodos
Lec_Esc::Lec_Esc() {

    escrib = false;
    n_lec = 0;
    lectura = newCondVar();
    escritura = newCondVar();
}

void Lec_Esc::ini_lectura() {

    // Si estan escribiendo, esperamos
    if ( escrib )
        lectura.wait();

    n_lec++;

    const int aux = n_lec;
    cout << "Empieza lectura (" << aux << ")" << endl;

    lectura.signal();
}

void Lec_Esc::ini_escritura() {

    // si todavia hay lectores escribiendo esperamos
    if ( escrib or n_lec > 0 )
        escritura.wait();

    escrib = true;

    cout << "Empieza escritura" << endl;
}

void Lec_Esc::fin_lectura() {

    // ha terminado de leer, menos un lector
    n_lec--;
    assert( n_lec >= 0 );

    const int aux = n_lec;
    cout << "Termina lectura (" << aux << ")" << endl;

    // si es el ultimo lector, señala a un escritor
    if ( n_lec == 0 )
        escritura.signal();
}

void Lec_Esc::fin_escritura() {

    escrib = false;

    cout << "Termina escritura" << endl;

    // Si hay lectores leyendo
    if ( not lectura.empty() )
        // Señalamos a otro lector
        lectura.signal();
    else
        // Sino, mpezamos a escribir
        escritura.signal();
}
/////////////////////////////////////////////////////////////////////////////////////////////

// Para mas legibilidad
void espera() { this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ) ); }

/////////////////////////////////////////////////////////////////////////////////////////////
// procedimientos de hebras
void funcion_hebra_lector( MRef<Lec_Esc> monitor ) {

    while (true) {
        monitor->ini_lectura();
        espera();
        monitor->fin_lectura();
        espera();
    }
}

void funcion_hebra_escritor( MRef<Lec_Esc> monitor ) {

    while (true) {
        monitor->ini_escritura();
        espera();
        monitor->fin_escritura();
        espera();
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////
int main() {

    // Lectores y escritores, respectivamente
    const unsigned int n = 3, m = 4;

    MRef<Lec_Esc> monitor = Create<Lec_Esc>();

    // Inicializamos las hebras
    thread lectores[n];
    for (int i=0; i < n; i++)
        lectores[i] = thread(funcion_hebra_lector, monitor);

    thread escritores[m];
    for (int i=0; i < m; i++)
        escritores[i] = thread(funcion_hebra_escritor, monitor);

    // Y "esperamos su finalizacion"
    for (int i=0; i < n; i++)
        lectores[i].join();
    for (int i=0; i < m; i++) {
        escritores[i].join();
    }

    return 0;
}