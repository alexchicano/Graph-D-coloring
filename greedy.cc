/**
 * @file: greedy.cc
 * @autors: Alex Chicano i Marc Rovira
 *
 * Algorisme greedy per obtenir una D coloració localment mínima:
 *  1) Cridem la funció ordenacio() que retorna el vector ordre.
 *  2) Recorrem les pel·lícules segons l'ordre obtingut i assignem el mínim color disponible als vértexs adjacents de cada pel·lícula.
 *  
 */
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>
#include <math.h>
#include <iomanip>
using namespace std;

vector<string> I2P, I2S;   // Vector que guarda les pel·lícules i les sales
map<string, int> P2I, S2I; // Diccionari pel·lícules i sales
vector<vector<int>> G;     // Llista d'adjacència
vector<int> ordre;         // Vector d'ordenació de pel·lícules per total de restriccions
vector<int> color;         // Vector que emmagatzema la col·loració actual (assignació de pel·lícules a un dia)

string name; // Nom del fitxer resultant (introduït per l'usuari)
double S;    // Sales --> màxim de vèrtexs d'un mateix color
double P;    // Pel·lícules --> nombre de vèrtexs
int D;       // Dies --> Nombre cromàtic (mín nombre de dies necessaris)

double t0;

void read_instance(const char* file) {
  ifstream in(file);

  in >> P;
  I2P.resize(P);
  for (int k = 0; k < P; ++k) {
    in >> I2P[k];
    P2I[ I2P[k] ] = k;
  }

  G.resize(P);
  int L;
  in >> L;
  for (int k = 0; k < L; ++k) {
    string sp, sq;
    in >> sp >> sq;
    assert(P2I.count(sp));
    assert(P2I.count(sq));
    int p = P2I[sp];
    int q = P2I[sq];
    G[p].push_back(q);
    G[q].push_back(p);
  }
  
  in >> S;
  I2S.resize(S);
  for (int k = 0; k < S; ++k) {
    in >> I2S[k];
    S2I[ I2S[k] ] = k;
  }
}

// Guarda en un fitxer els valors resultants de la millor solució trobada fins al moment
void save() {
  double t1 = clock();
  // Calcula el temps emprat per trobar la solució
  double time = (double(t1-t0)/CLOCKS_PER_SEC);

  ofstream myfile;
  myfile.open(name.c_str(), ios::out);

  // Escriu el temps amb un sol decimal
  myfile << fixed << setprecision(1) << time << endl;

  // Escriu el nombre de dies òptims (valor de la funció objectiu) de la solució
  myfile << D << endl;

  // Escriu una línia per cadascuna de les pel·lícules que conté (nom, dia, sala)
  int s;
  for(int i = 0; i < D; ++i) {
    s = 0;
    for(int j = 0; j < P; ++j) {
      if (color[j] == i) {
        myfile << I2P[j] << " " << i+1 << " " << I2S[s] << endl;
        ++s;
      }
    }
  }
  myfile.close();
}

// Funció que retorna un vector ordre el qual conté els índexs de les pel·lícules ordenats segons els seu nombre de restriccions (de forma decreixent)
void ordenacio() {
  vector<int> total_restr(P);
  for(int v = 0; v < P; v++) {
    total_restr[v] = G[v].size();
    ordre[v] = v;
  }
  sort(ordre.begin(), ordre.end(), [&](int a, int b) -> bool { return total_restr[a] > total_restr[b]; });
}

// Funció que implementa l'algorisme greedy descrit al principi
void graphColoring() {
  ordre = vector<int> (P);
  // 1)
  ordenacio();
  color = vector<int> (P, -1);
  
  D = 0; // Emmagatzema el major color utilitzat
  vector<int> used(P); // used[d] indica el nombre de pel·lícules projectades el dia "d"
  
  // 2)
  for(int v = 0; v < P; v++) {
    vector<bool> adj_colored(P, false); 
    int node = ordre[v];
    // Marquem els nodes adjacents que han estat colorejats.
    for (int i : G[node]) {
      if (color[i] != -1) adj_colored[color[i]] = true;
    }
    // Busquem el 1r color disponible i que no execedeix del nombre de sales
    int c = 0;
    while (adj_colored[c] or used[c] == S) ++c;
    color[node] = c;
    ++used[c];
    D = max(D, c); 
  }
  ++D; // Comencem amb el color 0
}

int main(int argc, char** argv) {

  if (argc != 3) {
    cout << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
    exit(0);
  }

  t0 = clock();

  read_instance(argv[1]);
  name = argv[2];

  graphColoring();
  save();
}

/* 
Execució:
g++ -Wall -O2 -std=c++17 -o greedy greedy.cc
./greedy instances/easy-1.inp result.txt
./greedy instances/med-3.inp result.txt
*/