//		CARLOS SÁNCHEZ PÁEZ
//					2ºA (A2)
//		EXAMEN P3 11-12-17
//		DNI: 25613096-C
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
num_filosofos = 5 ,
num_procesos  = num_filosofos + 1 ;   // El +1 es por el camarero.

const int ID_CAMARERO = num_procesos - 1;	//El camarero es el último
const int MAX_SENTADOS = 4;

const int etiq_COGER = 0, etiq_SOLTAR = 1;


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

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int peticion;
  MPI_Status estado;
  while ( true )
  {
    //Nos sentamos
    cout << "Filosofo " << id << " se sienta." << endl;

    cout << "Filósofo " << id << " solicita tenedores" << endl<<flush;
    MPI_Ssend(&peticion, 1, MPI_INT, ID_CAMARERO, etiq_COGER, MPI_COMM_WORLD); // ... solicitar tenedor izquierdo (completar)


    cout << "Filósofo " << id << " comienza a comer" << endl<<flush ;
    sleep_for( milliseconds( aleatorio<10, 100>() ) );

    cout << "Filósofo " << id << " va a soltar sus tenedores." << endl<<flush;
    MPI_Ssend(&peticion, 1, MPI_INT, ID_CAMARERO, etiq_SOLTAR, MPI_COMM_WORLD);// ... soltar el tenedor izquierdo (completar)


    //Nos levantamos
    cout << "Filosofo " << id << " se levanta." << endl<<flush;


    cout << "Filosofo " << id << " comienza a pensar" << endl<<flush;
    sleep_for( milliseconds( aleatorio<10, 100>() ) );
  }
}
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------

void funcion_camarero() {
  int peticion, confirmacion;
  int tenedores_libres=5;
  int cliente;
	string cadena;
  MPI_Status estado;
  while (true) {
    if (tenedores_libres >= 2)   //Los filósofos pueden coger o soltar los tenedores.
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
    else                         // Los filósofos sólo pueden soltar los tenedores y levantarse. 
      MPI_Probe(MPI_ANY_SOURCE, etiq_SOLTAR, MPI_COMM_WORLD, &estado);           //Sólo sondeo, no recibo.

    cliente = estado.MPI_SOURCE;

    if (estado.MPI_TAG == etiq_SOLTAR) {
      MPI_Recv(&peticion, 1, MPI_INT, cliente, etiq_SOLTAR, MPI_COMM_WORLD, &estado);
      if (cliente == 1 || cliente == 3){			//Primitivos
	    tenedores_libres++;
	    cadena = "primitivo ";
      }
      else{
	    tenedores_libres+=2;
	    cadena="";
      }
      cout << "Filosofo " << cadena << cliente << " suelta sus tenedores. Quedan " << tenedores_libres << " tenedores libres." << endl<<flush;
    }
    else if (estado.MPI_TAG == etiq_COGER) {
      MPI_Recv(&peticion, 1, MPI_INT, cliente, etiq_COGER, MPI_COMM_WORLD, &estado);
      if (cliente == 1 || cliente == 3){		//Si son los filósofos primitivos, se le da un tenedor.
          tenedores_libres--;
          cadena="primitivo ";
      }
      else {
          tenedores_libres-=2;
	  cadena="";
      }
      cout << "Filosofo " << cadena << cliente << " coge sus tenedores. Hay " << tenedores_libres << " tenedores libres." << endl<<flush;

    }
  }
}



// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
  int id_propio, num_procesos_actual ;

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
  MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


  if ( num_procesos == num_procesos_actual )
  {
    // ejecutar la función correspondiente a 'id_propio'
    if (id_propio == ID_CAMARERO)
      funcion_camarero();
    else 
      funcion_filosofos( id_propio ); //   es un filósofo
  }
  else
  {
    if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
    { cout << "el número de procesos esperados es:    " << num_procesos << endl
           << "el número de procesos en ejecución es: " << num_procesos_actual << endl
           << "(programa abortado)" << endl ;
    }
  }

  MPI_Finalize( );
  return 0;
}

// ---------------------------------------------------------------------
