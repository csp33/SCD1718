/**************************** /
	Carlos Sánchez Páez
			2A (A2)
		DNI: 25613096-C
/*****************************/

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

const int NUM_FUM = 3;

class Estanco : public HoareMonitor {
private:
	CondVar estanquero;
	CondVar fumadores[NUM_FUM];	
	int ingrediente_anterior;
	bool mostrador_vacio;
	int contadores[NUM_FUM];
public:
	Estanco();
	void ObtenerIngrediente(int i);
	void PonerIngrediente(int i);
	void esperarRecogidaIngrediente();

};


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
//
int producirIngrediente () {
	chrono::milliseconds espera_ing( aleatorio<20, 200>() );
	this_thread::sleep_for( espera_ing );
	return aleatorio < 0, NUM_FUM - 1 > ();

}
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef <Estanco> e )
{
	while (true) {
		int ingrediente;
		while (true) {
			ingrediente = producirIngrediente ();
			e->PonerIngrediente(ingrediente);
			e->esperarRecogidaIngrediente();
		}
	}
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

	// calcular milisegundos aleatorios de duración de la acción de fumar)
	chrono::milliseconds duracion_fumar( aleatorio<20, 200>() );

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
void  funcion_hebra_fumador( int num_fumador, MRef <Estanco> e)
{
	while ( true )
	{
		e->ObtenerIngrediente(num_fumador);
		fumar(num_fumador);
	}
}


//----------------------------------------------------------------------

int main()
{

//Declaración de hebras
	MRef<Estanco> monitor = Create<Estanco>();
	thread estanquero;
	thread fumadores[NUM_FUM];

//Ponemos las hebras a trabajar

	estanquero = thread(funcion_hebra_estanquero, monitor);
	for (int i = 0; i < NUM_FUM; i++)
		fumadores[i] = thread(funcion_hebra_fumador, i, monitor);

//Hacemos join a las hebras

	estanquero.join();
	for (int i = 0; i < NUM_FUM; i++)
		fumadores[i].join();

}
//----------------------------------------------------------------------


Estanco::Estanco() {
	mostrador_vacio=false;
	estanquero = newCondVar();
	for (int i = 0; i < NUM_FUM; i++){
		contadores[i]=0;
		fumadores[i] = newCondVar();
	}
	ingrediente_anterior = 3;		//Ponemos uno no válido para que no comience ningun fumador.
}
void Estanco::ObtenerIngrediente(int i) {
	if (i != this->ingrediente_anterior)
		fumadores[i].wait();
	contadores[i]++;
	mostrador_vacio=true;
	cout<<"El fumador "<<i<<" retira su ingrediente."<<endl;
	if(contadores[i] != 2){		//Todavía no tengo los dos.
		estanquero.signal();
		fumadores[i].wait();
	}
	contadores[i]=0;
	estanquero.signal();
}
void Estanco::PonerIngrediente(int i) {
	if( i == this->ingrediente_anterior)
		cout<<"Estanquero ha puesto el mismo ingrediente que antes."<<endl;
	this->ingrediente_anterior = i;
	mostrador_vacio=false;
	cout << "El estanquero pone el ingrediente " << i << " en el mostrador." << endl;
	fumadores[ingrediente_anterior].signal();

}
void Estanco::esperarRecogidaIngrediente() {
	if (!mostrador_vacio)
		estanquero.wait();
}

