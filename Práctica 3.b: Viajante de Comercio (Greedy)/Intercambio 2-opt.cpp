//////////////////////////////////////////////////////////////////
// Álvaro Fernández García
// Alexandra-Diana Dumitru
// Ana María Romero Delgado
// Germán Castro Montes
// Marino Fajardo Torres
// NOTA: compilar con -std=c++11
/////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <vector>

using namespace std;

//Función para leer los puntos del archivo:
void leer_puntos(string & nombre, map<int,pair<double,double>> & M){
    ifstream datos;
    string s;
    pair<double,double> p;
    int n,act;

    datos.open(nombre.c_str());
    if (datos.is_open()) {
   		datos >> s; // Leo "dimension:"
  		datos >> n;
  	  	int i=0;

		while(i<n){
    	    datos >> act >> p.first >> p.second;
			M[act] = p;
			i++;
    	}

   		datos.close();
   }
   else
      cout << "Error de Lectura en " << nombre << endl;
}


//Función para calcular la distancia euclídea entre dos puntos:
double Distancia(pair<double,double> p1, pair<double,double> p2){
	return sqrt(pow(p2.first-p1.first,2) + pow(p2.second-p1.second,2));
}


//Función para calcular la matriz de distancias:
void CalculaMD(double** M, int n, map<int,pair<double,double>> & mapa){
	for(int i = 1; i<n; ++i)
		for(int j = 1; j<n; ++j)
			M[i][j] = Distancia(mapa[i], mapa[j]);
}

//Método para calcular el coste de una solución:
double coste(vector<int> Solucion, double** MD){
  double coste = 0;
  for(unsigned int i = 0; i < Solucion.size()-1; ++i)
    coste += MD[Solucion[i]][Solucion[i+1]];

  coste += MD[Solucion.front()][Solucion.back()];

  return coste;
}

//Método para calcular un recorrido para el viajante de comercio basado en el vecino más cercano:
pair<vector<int>,double> VecinoMasCercano(vector<int> & candidatos, double** MD){
	
	//Variables para el calculo de la distancia;
	vector<int> S, Stemp, candidatos_tmp;
	double MejorDist = -1.0, distTemp, min_dist;
	int c_actual, c_siguiente;

	//Calcular la distancia partiendo de distintas ciudades:
	for(int i = 0; i < candidatos.size(); ++i){
		
		distTemp = 0.0;
		candidatos_tmp = candidatos;
		c_actual = candidatos_tmp[i];
		Stemp.push_back(c_actual); //Añadir a la lista de seleccionados.
		candidatos_tmp.erase(candidatos_tmp.begin()+i); //Borrar de entre los candidatos la ciudad.

		while(!candidatos_tmp.empty()){
			
			//Función Selección. El vecino más cercano:
			min_dist = MD[c_actual][candidatos_tmp[0]];
			c_siguiente = 0;
			for(int j = 1; j < candidatos_tmp.size(); ++j)
				if(MD[c_actual][candidatos_tmp[j]] < min_dist){
					c_siguiente = j;
					min_dist = MD[c_actual][candidatos_tmp[j]];
				}	
			//Las ciudades elegidas son factibles, ya que las ya elegidas no figuran en candidatos.
			distTemp += min_dist;
			Stemp.push_back(candidatos_tmp[c_siguiente]);
			c_actual = candidatos_tmp[c_siguiente];
			candidatos_tmp.erase(candidatos_tmp.begin()+c_siguiente);
		}

		//Añadir a distTemp la distancia entre la primera y la última ciudad:
		distTemp += MD[Stemp.front()][Stemp.back()];

		//Comprobar si es mejor que la que ya había:
		if(MejorDist == -1.0){
			MejorDist = distTemp;
			S = Stemp;
		}
		else if(distTemp < MejorDist){
			MejorDist = distTemp;
			S = Stemp;
		}
		Stemp.clear(); // Vaciar la solución temporal.
	}

	return pair<vector<int>,double>(S, MejorDist);
}

//Método para calcular un recorrido para el viajante de comercio basado en el intercambio de aristas 2-opt:
pair<vector<int>,double> ViajanteDeComercioIntercambio(vector<int> & candidatos, double** MD){

  int aux;
  unsigned int pos;
  vector<int> S_aux, S;
  double coste_aux;
  bool mejor_enc, cambio_realizado = true;

  //Considerar una solución inicial:
  //En este caso utilizaremos el método del vecino más cercano:
  auto SolInicio = VecinoMasCercano(candidatos, MD);
  S = SolInicio.first;
  double mejor_coste = SolInicio.second;

  while(cambio_realizado){
    cambio_realizado = false;
    for(unsigned int i = 0; i < S.size()-1; ++i){
      mejor_enc = false;
      //Examinar los movimientos 2-opt que incluyan la arista de i a su sucesor en el ciclo:
      for(unsigned int j = i + 1; j < S.size(); ++j){
        S_aux = S;
        //Intercambiar
        aux = S_aux[i];
        S_aux[i] = S_aux[j];
        S_aux[j] = aux;
        //Calcular el coste de la nueva solución:
        coste_aux = coste(S_aux, MD);
        //Si es mejor guardo:
        if(coste_aux < mejor_coste){
          mejor_enc = true;
          pos = j;
          mejor_coste = coste_aux;
        }
      }
      //Si se ha encontrado algo mejor, intercamio:
      if(mejor_enc){
        cambio_realizado = true;
        aux = S[i];
        S[i] = S[pos];
        S[pos] = aux;
      }
    }
  }
  return pair<vector<int>, double>(S, mejor_coste);
}

int main(int argc, char* argv[]){

	if(argc < 2){
		cout << "Modo de empleo: " << argv[0] << " <archivo.tsp>" << endl;
		exit(-1);
	}

	string archivo(argv[1]);
	map<int,pair<double,double>> puntos;

	//Leer los puntos:
	leer_puntos(archivo, puntos);
	int N = puntos.size();

	//Reservar matriz de distancia:
	double** D = new double*[N+1];
	for(int i = 0; i<N+1; ++i)
		D[i] = new double[N+1];

	//Calcular matriz de Distancias
	CalculaMD(D, N+1, puntos);

	//Construir vector de candidatos:
	vector<int> candidatos;
	for(int i = 0; i<N; ++i)
		candidatos.push_back(i+1);

	//Calcular recorrido:
	pair<vector<int>,double> res = ViajanteDeComercioIntercambio(candidatos, D);

	//Imprimir resultados:
	cout << "Orden de recorrido: ";
	for(unsigned int i = 0; i < res.first.size(); ++i)
		cout << res.first[i] << " ";
	cout << endl;

	cout << "Distancia: " << res.second << endl;

	for(int i = 0; i<N+1; ++i)
		delete[] D[i];
	delete[] D;

	exit(0);
}
