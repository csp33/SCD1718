#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

const int NUM_COCHES = 10;

template< int min, int max > int aleatorio()
{
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

void esperaAleatoria() {
	chrono::milliseconds espera( aleatorio<50, 250>() );
	this_thread::sleep_for( espera );
}


/*
	Es un paso estrecho en el que hay dos extremos: izquierdo y derecho.
	El invariante es que sólo puede circular un vehículo simultáneamente por el paso.
	Los coches se irán alternando: uno del lado izquierdo, otro del lado derecho...
*/

class PasoEstrecho : public HoareMonitor {
private:
	CondVar ladoIzquierdo;
	CondVar ladoDerecho;
	bool alguien;
public:
	PasoEstrecho();
	void EntraIzquierdo(int i);
	void EntraDerecho(int i);
	void SaleIzquierdo(int i);
	void SaleDerecho(int i);
};

PasoEstrecho::PasoEstrecho() {
	ladoIzquierdo = newCondVar();
	ladoDerecho = newCondVar();
	alguien = false;
}

void PasoEstrecho::EntraIzquierdo(int i) {
	if (alguien)
		ladoIzquierdo.wait();
	cout << "Entra el coche " << i << " por el lado izquierdo." << endl;
	alguien = true;

}

void PasoEstrecho::EntraDerecho(int i) {
	if (alguien)
		ladoDerecho.wait();
	cout << "Entra el coche " << i << " por el lado derecho." << endl;
	alguien = true;

}
void PasoEstrecho::SaleIzquierdo(int i) {
	cout << "Sale el coche " << i << " por el lado izquierdo." << endl;
	alguien = false;
	ladoIzquierdo.signal();

}

void PasoEstrecho::SaleDerecho(int i) {
	cout << "Sale el coche " << i << " por el lado derecho." << endl;
	alguien = false;
	ladoDerecho.signal();

}


void funcion_hebra_izquierdo(MRef<PasoEstrecho> m, int i) {
	while (true) {
		m->EntraIzquierdo(i);
		esperaAleatoria();
		m->SaleDerecho(i);
	}
}

void funcion_hebra_derecho (MRef<PasoEstrecho> m, int i) {
	while (true) {
		m->EntraDerecho(i);
		esperaAleatoria();
		m->SaleIzquierdo(i);
	}
}

int main() {

	MRef<PasoEstrecho> m = Create<PasoEstrecho> ();
	thread izq[NUM_COCHES];
	thread dcho[NUM_COCHES];

	for (int i = 0; i < NUM_COCHES; i++) {
		izq[i] = thread(funcion_hebra_izquierdo, m, i);
		dcho[i] = thread(funcion_hebra_derecho, m, i);
	}

	for (int i = 0; i < NUM_COCHES; i++) {
		izq[i].join();
		dcho[i].join();
	}

}
