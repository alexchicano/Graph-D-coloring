#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <math.h>
#include <iomanip>
using namespace std;

vector<string> I2P, I2S;   // Vector que guarda les pel·lícules i les sales
map<string, int> P2I, S2I; // Diccionari pel·lícules i sales
vector<vector<int>> G;     // Llista d'adjacència
vector<int> ordre;

string name; // Nom del fitxer resultant (introduït per l'usuari)
double S;    // Sales --> màxim de vèrtexs d'un mateix color
double P;    // Pel·lícules --> nombre de vèrtexs
int D;       // Dies --> Nombre cromàtic (mín # dies necessaris)
//int L;     // Restriccions --> Arestes

double t0;   // Temps inicial d'execució

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

void save (const vector<int>& color) {

  double t1 = clock();
  double time = (double(t1-t0)/CLOCKS_PER_SEC);

  ofstream myfile;
  myfile.open(name.c_str(), ios::out);

  // Escriu el temps emprat per trobar la solució
  myfile << fixed << setprecision(1) << time << endl;

  // Escriu el nombre de dies òptims (valor de la funcio objectiu) de la sol·lució
  myfile << D << endl;

  // Escriu una ́ınia per cadascuna de les pel·lícules (nom, dia, sala)
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

// Comprova si l'assignació de colors actual és correcta per al vèrtex v.
bool legal(int v, const vector<int>& color, int cv, const vector<int>& used) {
  for (int c = 0; c < D; c++) if (used[c] > S) return false;
  for(int i : G[v]) if (cv == color[i]) return false;
  return true;
}

// Backtracking per decidir si es pot obtenir una D coloració
bool graphColoring(vector<int>& color, int v, vector<int>& used) {
  int node = ordre[v];
  if (v == P) {
    for (int c = 0; c < D; c++) if (used[c] > S) return false;
    return true;
  } 
  // Considerem el vèrtex v i provem diferents colors
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

void ordenacio() {
  vector<int> total_restr(P);
  for(int v = 0; v < P; v++) {
    total_restr[v] = G[v].size();
    ordre[v] = v;
  }
  sort(ordre.begin(), ordre.end(), [&](int a, int b) -> bool { return total_restr[a] > total_restr[b]; });
}

bool check_D() {
  cout << D << endl;
  vector<int> used(D, 0);
  vector<int> color(P, -1);
  if (graphColoring(color, 0, used)) {
    save(color);
    return true;
  }
  cout << "Used :" << endl;
  for (int k : used) cout << k << " "; 
  cout << endl;
  cout << "Colors :" << endl;
  for (int c : color) cout << c << " "; 
  cout << endl;
  return false;
}

// Cerca binària per trobar el nombre cromàtic --> mínim nombre de dies del festival

void festival_duration(double D_min, double D_max) {

  D = round((D_max+D_min)/2);
  cout << "D: " << D << "  D_min: " << D_min << "  D_max: " << D_max << endl;

  // BASE: Quan l'interval de D a biseccionar sigui de longitud 1 (o 0 si S = 1)
  if (D_max-D_min == 1 or D_max-D_min == 0) {
    
    D = D_min; // Prova amb D_min abans ja que busquem el mínim D possible
    cout << "D: " << D << "  D_min: " << D_min << "  D_max: " << D_max << endl;
    if (check_D()) return; 
    
    D = D_max;
    if (check_D()) return;  // Si no, prova amb D_max
        
  }

  // RESUCRSIU: decideix sobre quina meitat de l'interval buscar
  else {
    if (check_D()) D_max = D;
    else D_min = D;
    festival_duration(D_min, D_max);
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
    exit(0);
  }

  t0 = clock();

  read_instance(argv[1]);
  name = argv[2];

  ordre = vector<int> (P);
  ordenacio();
  //color = vector<int>(P, -1); //UNUSED node initialized as -1
  

  festival_duration(ceil(P/S), P);
  cout << "El nombre de dies és : " << D << endl;
}


/* 
Execució:
g++ -Wall -O2 -std=c++17 -o exh_cerca_binaria exh_cerca_binaria.cc
./exh_cerca_binaria instances/easy-9.inp result.txt
./exh_cerca_binaria instances/med-3.inp result.txt
*/
