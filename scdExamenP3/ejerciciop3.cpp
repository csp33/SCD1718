//		CARLOS SÁNCHEZ PÁEZ
//					2ºA (A2)
//		EXAMEN P3 11-12-17
//		DNI: --------------
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int num_alumnos = 9;
const int num_procesos = num_alumnos + 2;		//Bareto y coctelero

const int ID_BARETO=num_procesos-2;
const int ID_COCTELERO=num_procesos - 1;

const int etiq_UNA=0,etiq_DOS=1,etiq_CONF=2,etiq_RELLENAR=3;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void esperar(){
    sleep_for( milliseconds( aleatorio<10, 100>() ) );
}

void funcion_alumno(int id){
    int num_copas;
    int confirmacion;
    int etiqueta;
    MPI_Status estado;
    while(true){
	cout<<"El alumno "<<id<<" va camino al bar."<<endl<<flush;
	esperar();
 	num_copas=aleatorio<1,2>();
 	if (num_copas == 1)		//Asigno etiquetas
	    etiqueta=etiq_UNA;
	else
 	    etiqueta=etiq_DOS;
 	cout<<"El alumno "<<id<<" decide tomarse "<< num_copas <<" copas."<<endl<<flush;
 	MPI_Ssend(&num_copas,1,MPI_INT,ID_BARETO,etiqueta,MPI_COMM_WORLD);		//Envío petición
	cout<<"El alumno "<<id<<" solicita al bareto "<<num_copas<<" copas."<<endl<<flush;
	MPI_Recv(&confirmacion,1,MPI_INT,ID_BARETO,etiq_CONF,MPI_COMM_WORLD,&estado);	//Recibo confirmación
	cout<<"El alumno "<<id<<" bebe sus copas."<<endl<<flush;
	esperar();
	cout<<"El alumno "<<id<<" sale del bar."<<endl<<flush;
	esperar();
    }
}

void funcion_bareto(){
	int copas_en_barra=0;
	int valor;
	int peticion;
	int emisor;
	int confirmacion;
	MPI_Status estado;
	while(true){
	    if (copas_en_barra >= 3)		//Puedo recibir cualquier mensaje
		MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&estado);
	    else if (copas_en_barra>=1)	//Solo pueden beber una
		MPI_Probe(MPI_ANY_SOURCE,etiq_UNA,MPI_COMM_WORLD,&estado);
	    else{										//No hay copas
		MPI_Ssend(&peticion,1,MPI_INT,ID_COCTELERO,etiq_RELLENAR,MPI_COMM_WORLD);
		MPI_Probe(ID_COCTELERO,etiq_CONF,MPI_COMM_WORLD,&estado);
	    }
	    emisor=estado.MPI_SOURCE;
	    MPI_Recv(&valor,1,MPI_INT,emisor,estado.MPI_TAG,MPI_COMM_WORLD,&estado);
	    if (emisor == ID_COCTELERO){		//Es el coctelero
		copas_en_barra+=6;
		cout<<"El coctelero repone existencias. Hay "<< copas_en_barra <<" copas en la barra."<<endl<<flush;
	    }
	    else{						//Es un alumno
		copas_en_barra-=valor;
		cout<<"El alumno "<<emisor<<" coge "<<valor<<" copas. Quedan "<< copas_en_barra <<" copas en la barra."<<endl<<flush;
		MPI_Ssend(&confirmacion,1,MPI_INT,emisor,etiq_CONF,MPI_COMM_WORLD);
	    }

    }
}

void funcion_coctelero(){
    int peticion;
    int confirmacion;
     MPI_Status estado;
    while(true){
	MPI_Recv(&peticion,1,MPI_INT,ID_BARETO,etiq_RELLENAR,MPI_COMM_WORLD,&estado);	//Recibo petición
	cout<<"El coctelero prepara la mercancía para reponerla."<<endl<<flush;
	esperar();
	MPI_Ssend(&confirmacion,1,MPI_INT,ID_BARETO,etiq_CONF,MPI_COMM_WORLD);
    }
}



int main(int argc, char **argv){

  int id_propio, num_procesos_actual ;
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
  MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

  if ( num_procesos == num_procesos_actual )		//Si hemos ejecutado bien, lanzamos los procesos según su rol.
  {
	if(id_propio == ID_BARETO)
	    funcion_bareto();
	else if (id_propio == ID_COCTELERO)
	    funcion_coctelero();
	else
	    funcion_alumno(id_propio);
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
