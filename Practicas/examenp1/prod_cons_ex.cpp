/*

Carlos Sánchez Páez
2A
DNI:25613096C

*/
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
	       tam_vec_1   = 30 ,   // tamaño del buffer
		tam_vec_2 = 10;
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int vector1[tam_vec_1];
int vector2[tam_vec_2];
int prod_cons=0;
int indice_v1=0,indice_v2=0;
Semaphore puede_producir=tam_vec_1;
Semaphore puede_consumir=0;
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
/**************Función del examen*********************************/
void hebra_intermedia(){
	cout<<"El valor de la variable prod_cons es "<<prod_cons<<endl;
	int dato=vector1[indice_v1];
	if(dato%3==0)
		dato*=10;
	vector2[indice_v2]=dato;
	indice_v1++;
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
   assert( dato < num_items*10 );
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
	if(dato%2==0)
		prod_cons++;
	else
		prod_cons-=2;
	sem_wait(puede_producir);
	vector1[indice_v1]=dato;
	hebra_intermedia();
	indice_v1--;
	sem_signal(puede_consumir);	//Damos luz verde al consumidor
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
	int dato ;
	sem_wait(puede_consumir);
	dato=vector2[indice_v2];
	indice_v2--;
	sem_signal(puede_producir);
	consumir_dato( dato ) ;
	if(dato%2==0)
		prod_cons++;
	else
		prod_cons--;
    }
}
//----------------------------------------------------------------------





//----------------------------------------------------------------------

int main(){
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

  // test_contadores();
}
