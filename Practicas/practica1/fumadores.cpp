#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


Semaphore tiene_ingrediente[]={0,0,0};		 //Ninguno tiene el ingrediente necesario
Semaphore puede_producir=1;			 //En principio el mostrador está vacío
const int NUM_FUM=3;



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
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
	while(true){
		sem_wait(puede_producir);
		int elemento=aleatorio<0,NUM_FUM-1>();
		cout<<"El estanquero produce el ingrediente "<<elemento<<endl;
		sem_signal(tiene_ingrediente[elemento]);
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
	sem_wait(tiene_ingrediente[num_fumador]);
	cout<<"El fumador "<<num_fumador<<" retira su ingrediente."<<endl;
	sem_signal(puede_producir);
	fumar(num_fumador);
   }
}

//----------------------------------------------------------------------
//Función que hace join a un vector de hebras
void join_vector(thread *vector,int tam){
	for(int i=0;i<tam;i++)
		vector[i].join();
}

//----------------------------------------------------------------------
//Función que inicializa las hebras de los fumadores
void init_fum(thread *vector,int tam){
	for(int i=0;i<NUM_FUM;i++)
		vector[i]=thread(funcion_hebra_fumador,i);
}

//----------------------------------------------------------------------

int main()
{

//Declaración de hebras

	thread estanquero;
	thread fumadores[NUM_FUM];

//Ponemos las hebras a trabajar

	init_fum(fumadores,NUM_FUM);
	estanquero=thread(funcion_hebra_estanquero);

//Hacemos join a las hebras

	estanquero.join();
	join_vector(fumadores,NUM_FUM);

}
