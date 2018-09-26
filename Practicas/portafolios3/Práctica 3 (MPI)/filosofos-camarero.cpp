// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
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
num_procesos  = 2 * num_filosofos + 1 ;   // El +1 es por el camarero.

const int ID_CAMARERO = num_procesos - 1;
const int MAX_SENTADOS = 4;

const int etiq_COGER = 0, etiq_SOLTAR = 1;
const int etiq_SENTARSE = 2, etiq_LEVANTARSE = 3;


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
  int id_ten_izq = (id + 1)              % (num_procesos - 1), //id. tenedor izq.
      id_ten_der = (id + num_procesos - 2) % (num_procesos - 1); //id. tenedor der.
  int peticion, confirmacion;
  MPI_Status estado;
  while ( true )
  {
    //Pedimos sentarnos
    cout << "Filosofo " << id << " solicita sentarse." << endl;
    MPI_Ssend(&peticion, 1, MPI_INT, ID_CAMARERO, etiq_SENTARSE, MPI_COMM_WORLD);
    //Esperamos a recibir la confirmación
    MPI_Recv(&confirmacion, 1, MPI_INT, ID_CAMARERO, etiq_SENTARSE, MPI_COMM_WORLD, &estado);
    cout << "Filosofo " << id << " se sienta." << endl;

    cout << "Filósofo " << id << " solicita ten. izq." << id_ten_izq << endl;
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, etiq_COGER, MPI_COMM_WORLD); // ... solicitar tenedor izquierdo (completar)

    cout << "Filósofo " << id << " solicita ten. der." << id_ten_der << endl;
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, etiq_COGER, MPI_COMM_WORLD); // ... solicitar tenedor derecho (completar)

    cout << "Filósofo " << id << " comienza a comer" << endl ;
    sleep_for( milliseconds( aleatorio<10, 100>() ) );

    cout << "Filósofo " << id << " suelta ten. izq. " << id_ten_izq << endl;
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, etiq_SOLTAR, MPI_COMM_WORLD);// ... soltar el tenedor izquierdo (completar)

    cout << "Filósofo " << id << " suelta ten. der. " << id_ten_der << endl;
    MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, etiq_SOLTAR, MPI_COMM_WORLD);// ... soltar el tenedor derecho (completar)

    //Nos levantamos
    MPI_Ssend(&peticion, 1, MPI_INT, ID_CAMARERO, etiq_LEVANTARSE, MPI_COMM_WORLD);
    cout << "Filosofo " << id << " se levanta." << endl;


    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10, 100>() ) );
  }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
    MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_COGER, MPI_COMM_WORLD, &estado); // ...... recibir petición de cualquier filósofo (completar)
    id_filosofo = estado.MPI_SOURCE; // ...... guardar en 'id_filosofo' el id. del emisor (completar)
    cout << "Ten. " << id << " ha sido cogido por filo. " << id_filosofo << endl;

    MPI_Recv(&valor, 1, MPI_INT, id_filosofo, etiq_SOLTAR, MPI_COMM_WORLD, &estado); // ...... recibir liberación de filósofo 'id_filosofo' (completar)
    cout << "Ten. " << id << " ha sido liberado por filo. " << id_filosofo << endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero() {
  int actual = 0, peticion, confirmacion;
  int cliente;
  MPI_Status estado;
  while (true) {
    if (actual < MAX_SENTADOS)   //Los filósofos pueden sentarse o levantarse.
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
    else                         // Los filósofos sólo pueden levantarse.
      MPI_Probe(MPI_ANY_SOURCE, etiq_LEVANTARSE, MPI_COMM_WORLD, &estado);           //Sólo sondeo, no recibo.

    cliente = estado.MPI_SOURCE;

    if (estado.MPI_TAG == etiq_LEVANTARSE) {
      MPI_Recv(&peticion, 1, MPI_INT, cliente, etiq_LEVANTARSE, MPI_COMM_WORLD, &estado);
      actual--;
      cout << "Filosofo " << cliente << " se levanta. Quedan " << actual << " filósofos en la mesa." << endl;
    }
    else if (estado.MPI_TAG == etiq_SENTARSE) {
      MPI_Recv(&peticion, 1, MPI_INT, cliente, etiq_SENTARSE, MPI_COMM_WORLD, &estado);
      actual++;
      MPI_Ssend(&confirmacion, 1, MPI_INT, cliente, etiq_SENTARSE, MPI_COMM_WORLD);
      cout << "Filosofo " << cliente << " se sienta. Hay " << actual << " filósofos en la mesa." << endl;

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
    else if ( id_propio % 2 == 0 )          // si es par
      funcion_filosofos( id_propio ); //   es un filósofo
    else                               // si es impar
      funcion_tenedores( id_propio ); //   es un tenedor
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
