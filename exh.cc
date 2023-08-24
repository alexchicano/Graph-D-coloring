/**
 * @file: exh.cc
 * @autors: Alex Chicano i Marc Rovira
 *
 * Algorisme basat en cerca exhaustiva per obtenir una D coloració mínima
 *
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
void save(const vector<int>& color) {
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


// Comprova si l'assignació de colors actual és correcta donat un vèrtex (v) i el seu color (cv) 
bool legal(int v, const vector<int>& color, int cv, const vector<int>& used) {
  for (int c = 0; c < D; c++) {
    if (used[c] > S) return false;
  }
  for(int i : G[v]) if (cv == color[i]) return false;
  return true;
}

// Backtracking que intenta obtenir una D coloració, provant totes les assignacions possibles de colors en cada vértex
bool graphColoring(vector<int>& color, int v, vector<int>& used) {
  int node = ordre[v];
  // Un cop colorejat el graf, comprovar la restricció del nombre màxim de sales (nodes del mateix color)
  if (v == P) {
    for (int c = 0; c < D; c++) if (used[c] > S) return false;
    return true;
  } 
  // Considerem el vèrtex v i provem els diferents colors
  for(int c = 0; c < D; c++) {
    // Comprovem si l'assignació del color c a v és vàlida
    if (legal(node, color, c, used)) {
      color[node] = c;
      ++used[c];
      // Assignar recursivament colors a la resta de vèrtexs
      if (graphColoring(color, v + 1, used)) return true;
      // Retrocedim si amb c no s'obté una coloració vàlida
      color[node] = -1;
      --used[c];
    } 
  }
  return false;
}


// Cerca per trobar la duració mínima del festival
void festival_duration() {

  // Ordenem el vector de pel·lícules segons el nombre de restriccions per començar a assignar colors a les pel·lícules més conflictives
  ordre = vector<int> (P);
  ordenacio();
  
  D = P; // Pitjor cas per iniciar la cerca de D (tants dies com pel·lícules)
  int D_min = ceil(P/S); // Millor cas en el qual s'atura la cerca si s'assoleix 
  bool found = false;
  // Mentre no s'ha torbat la D mínima
  while (!found) {
    vector<int> used(D, 0);
    vector<int> color(P, -1); // UNUSED node inicialitzat com a -1
 
    if (graphColoring(color, 0, used)) {
      // Si la D-coloració no utilitza tots els colors (si hi ha algun 0 al vector used), restem a D els possibles colors no emprats
      D = D - count(used.begin(), used.end(), 0);
      save(color); // Si la D actual és vàlida, guardem la solució
      if (D > D_min) --D;
      else found = true;
    }
    else {
      // Si per aquesta D no hi ha coloració, atura la cerca
      found = true;
    }
  }
  return;
}


int main(int argc, char** argv) {

  if (argc != 3) {
    cout << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
    exit(0);
  }

  read_instance(argv[1]);
  name = argv[2];

  t0 = clock();
  festival_duration();
}


/* 
Execució:
g++ -Wall -O2 -std=c++17 -o exh exh.cc
./exh instances/easy-9.inp result.txt
./exh instances/med-3.inp result.txt
*/