#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>  // includes de MPI

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

/*****************************/

const int NUM_PROD = 4;
const int NUM_CONS = 5;

/***************************/

const int ELEMENTOS = NUM_PROD * NUM_CONS;
const int ID_BUFFER = NUM_PROD;


const int etiq_PRODUCTOR = 0;
const int etiq_CONSUMIDOR = 1;


template< int min, int max > int aleatorio()
{
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}


int producir(int id, const int K)
{
	static int contador = id * K ;
	sleep_for( milliseconds( aleatorio<50, 200>()) );
	contador++ ;
	cout << "Productor " << id << " ha producido valor " << contador << endl << flush;
	return contador ;
}

void consumir( int valor_cons , int id)
{
	// espera bloqueada
	sleep_for( milliseconds( aleatorio<10, 200>()) );
	cout << "Consumidor " << id % NUM_CONS << " ha consumido valor " << valor_cons << endl << flush ;
}

void funcion_productor(int id) {
	int k = ELEMENTOS / NUM_PROD;
	for (int i = 0; i < k; i++) {
		int producido = producir(id, k);
		cout << "Productor " << id << " va a enviar " << producido << endl;
		MPI_Ssend(&producido, 1, MPI_INT, ID_BUFFER, etiq_PRODUCTOR, MPI_COMM_WORLD);
	}
}

void funcion_consumidor(int id) {
	int peticion, recibido = 1;
	MPI_Status estado;
	for (int i = 0; i < ELEMENTOS / NUM_CONS; i++) {
		MPI_Ssend(&peticion, 1, MPI_INT, ID_BUFFER, etiq_CONSUMIDOR, MPI_COMM_WORLD);
		MPI_Recv(&recibido, 1, MPI_INT, ID_BUFFER, etiq_CONSUMIDOR, MPI_COMM_WORLD, &estado);
		cout << "Consumidor " << id % NUM_CONS << " recibe valor " << recibido << endl;
		consumir(recibido, id);
	}

}



void funcion_buffer() {
	const int TAM = 5;
	int buffer [TAM];
	int aceptado;
	int utilizados = 0;
	int valor;
	int peticion;
	int primera_libre = 0;
	int primera_ocupada = 0;
	MPI_Status estado;
	for (int i = 0; i < ELEMENTOS * 2; i++) {
		if (utilizados == 0)
			aceptado = etiq_PRODUCTOR;
		else if (utilizados == TAM)
			aceptado = etiq_CONSUMIDOR;
		else {
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);		//Espero a recibir algo.
			aceptado = estado.MPI_TAG;
		}

		//Una vez determinado el valor aceptado, hago el paso de mensajes.

		if (aceptado == etiq_PRODUCTOR) {				//Solicitud de productor
			MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_PRODUCTOR, MPI_COMM_WORLD, &estado);	//Recibo el valor
			buffer[primera_libre] = valor;															//Almaceno el valor
			primera_libre = (primera_libre + 1) % TAM;
			utilizados++;
			cout << "Buffer recibe " << valor << endl;
		}
		else if (aceptado == etiq_CONSUMIDOR) {		//Solicitud de consumidor
			MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiq_CONSUMIDOR, MPI_COMM_WORLD, &estado);	//Recibo la petición
			valor = buffer[primera_ocupada];
			primera_ocupada = (primera_ocupada + 1) % TAM;
			utilizados--;
			MPI_Ssend(&valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_CONSUMIDOR, MPI_COMM_WORLD);				//Envío el valor solicitado
			cout << "Buffer envia " << valor << endl;
		}
	}

}

const int ROL_PRODUCTOR = 0, ROL_CONSUMIDOR = 1, ROL_BUFFER = 2;


int miRolEs(const int id) {
	int devolver;
	if (id < NUM_PROD)
		devolver = ROL_PRODUCTOR;
	else if (id == NUM_PROD)
		devolver = ROL_BUFFER;
	else
		devolver = ROL_CONSUMIDOR;
	return devolver;
}

int main (int argc, char **argv) {
	int mi_id, num_proc;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &mi_id);
	if (num_proc != (NUM_PROD + NUM_CONS + 1)) {
		if (mi_id == 0)
			cout << "Esperados " << NUM_PROD + NUM_CONS + 1 << " procesos. Recibidos " << num_proc << endl;
	}
	else {

		int quienSoy;
		quienSoy = miRolEs(mi_id);
		if (quienSoy == ROL_PRODUCTOR)
			funcion_productor(mi_id);
		else if (quienSoy == ROL_CONSUMIDOR)
			funcion_consumidor(mi_id);
		else
			funcion_buffer();



	}
	MPI_Finalize();
}
