#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

const int NUM_CLIENTES = 10;

class Barberia : public HoareMonitor {
private:
	CondVar clientes;
	CondVar barbero;
	CondVar silla;
public:
	Barberia();
	void cortarPelo(int i);
	void siguienteCliente();
	void finCliente();
};

template< int min, int max > int aleatorio()
{
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

void esperaAleatoria() {
	chrono::milliseconds espera_ing( aleatorio<50, 250>() );
	this_thread::sleep_for( espera_ing );
}

void cortarPeloACliente() {
	esperaAleatoria();
}

void esperarFuera() {
	esperaAleatoria();
}

void funcion_hebra_barbero(MRef<Barberia> m) {
	while (true) {
		m->siguienteCliente();
		cortarPeloACliente();
		m->finCliente();
	}
}
void funcion_hebra_cliente(int i, MRef<Barberia> m) {
	while (true) {
		m->cortarPelo(i);
		esperarFuera();
	}
}

int main() {
	thread barbero, clientes[NUM_CLIENTES];
	MRef<Barberia> monitor = Create<Barberia>();

	//Inicializamos hebras
	barbero = thread(funcion_hebra_barbero, monitor);
	for (int i = 0; i < NUM_CLIENTES; i++)
		clientes[i] = thread(funcion_hebra_cliente, i, monitor);

	//Hacemos join
	barbero.join();
	for (int i = 0; i < NUM_CLIENTES; i++)
		clientes[i].join();
}
/***********************************************************/
Barberia::Barberia() {
	barbero = newCondVar();
	silla = newCondVar();
	clientes = newCondVar();
}

void Barberia::cortarPelo(int i) {
	if (!silla.empty())	{				//Si hay alguien en la silla
		cout << "El cliente " << i << " pasa a la sala de espera." << endl;
		clientes.wait();
	}
	cout << "El barbero comienza a pelar al cliente " << i << endl;
	barbero.signal();
	silla.wait();
}

void Barberia::siguienteCliente() {
	if (clientes.empty() && silla.empty()) {//Si no hay nadie, duerme.
		cout << "El barbero se pone a dormir." << endl;
		barbero.wait();
	}
	//Cuando lo despierten, llama a un cliente.
	clientes.signal();
}

void Barberia::finCliente() {
	cout << "El barbero termina de pelar al cliente." << endl;
	silla.signal();
}