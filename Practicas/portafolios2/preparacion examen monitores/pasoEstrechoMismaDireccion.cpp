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
	En esta modalidad se permite que circulen varios coches en la misma dirección.
*/

class PasoEstrecho : public HoareMonitor {
private:
	CondVar ladoIzquierdo;
	CondVar ladoDerecho;
	int izqADcha;
	int DchaAIzq;
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
	izqADcha = 0;
	DchaAIzq = 0;
}

void PasoEstrecho::EntraIzquierdo(int i) {
	if (DchaAIzq > 0)		//Si alguien pasa en sentido contrario, esperamos.
		ladoIzquierdo.wait();
	izqADcha++;
	cout << "Entra el coche " << i << " por el lado izquierdo." << endl;
}

void PasoEstrecho::EntraDerecho(int i) {
	if (izqADcha > 0)	//Si alguien pasa en sentido contrario...
		ladoDerecho.wait();
	DchaAIzq++;
	cout << "Entra el coche " << i << " por el lado derecho." << endl;

}
void PasoEstrecho::SaleIzquierdo(int i) {
	cout << "Sale el coche " << i << " por el lado izquierdo." << endl;
	DchaAIzq--;
	ladoDerecho.signal();
	ladoIzquierdo.signal();

}

void PasoEstrecho::SaleDerecho(int i) {
	cout << "Sale el coche " << i << " por el lado derecho." << endl;
	izqADcha--;
	ladoIzquierdo.signal();
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
