#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40 ,   // número de items
	       tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int buffer [tam_vec];	//Vector con el que trabajaremos
int posicion_leer=0;	//Posición en la que leerá el consumidor
int posicion_escribir=0;	//Posición en la que escribirá el productor
Semaphore puede_producir=tam_vec;	//Semáforo para el productor. Inicializado al tamaño del vector (puede producir hasta 10 elementos seguidos).
Semaphore puede_consumir=0;				//Semáforo para el consumidor. No puede consumir hasta que el productor produzca un dato.
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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
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
	sem_wait(puede_producir);					//Comprobamos si puede producir
	int dato = producir_dato() ;
	buffer[posicion_escribir]=dato;
  posicion_escribir++;
	if(posicion_escribir==10)
		posicion_escribir=0;
	sem_signal(puede_consumir);		  //Damos luz verde al consumidor
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
	int dato ;
	sem_wait(puede_consumir);				//Esperamos a que el productor produzca un dato
	dato=buffer[posicion_leer];
  posicion_leer++;
  if(posicion_leer==10)						//Si llegamos al final, ponemos la posición al inicio (cola circular)
    posicion_leer=0;

	sem_signal(puede_producir);		//Damos luz verde al productor
	consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main(){
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();
}
