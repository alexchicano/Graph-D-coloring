/**
 * @file: mh.cc
 * @autors: Alex Chicano i Marc Rovira

  PLANTEJAMENT: ALGORISME GENÈTIC
  Vam escollir aquesta metaheurística per l'interès què ens va despertar i perquè ens va donar molt bons resultats per les instàncies easy i med-1,2,7.

  Implementacions considerades:
  CROSS-OVERS:
  -Single point i Uniforme no han donat bons resultats donat la gran diferència entre individus.
  -Permutacions internes forçant millora sí que ha acabat donant millors resultats.

  MUTACIONS:
  -Mutació aleatòria no ha donat lloc a bons resultats.
  -Mutació pels individus que no s'aconsegueixen millorar amb permutacions sí que ha estat bona implementació.

  SELECCIONS:
  -Tournament Selection portava a molt poca variabilitat i l'estancament de la població.
  -Roulette wheel ha estat la que millor ha funcionat.

  Tot i les moltes hores dedicades i les diferents implementacions, el nostre algorisme no porta els individus dins d'un veinat suficientment proper a la solució l'òptima i considerem que és molt tard per canviar de metaheurística. No obstant, entreguem el nostre algorisme amb les idees que millor han funcionat tot intentant conservar l'esperit de l'algorisme genètic.
 
 */
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>
#include <math.h>
#include <iomanip>
#include <climits>
using namespace std;

vector<int> ordre;
vector<string> I2P, I2S;     // Vector que guarda les pel·lícules i les sales
map<string, int> P2I, S2I;   // Diccionari pel·lícules i sales
vector<vector<int>> G;       // Llista d'adjacència
vector<vector<int>> M_used;  // Matriu de colors utilitzats


string name; // Nom del fitxer resultant (introduït per l'usuari)
double S;    // Sales --> màxim de vèrtexs d'un mateix color
double P;    // Pel·lícules --> nombre de vèrtexs

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
void save(const vector<int>& color, int D) {
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
  int dies = 0;
  for(int i = 0; i < P; ++i) {
    s = 0;
    for(int j = 0; j < P; ++j) {
      if (color[j] == i) {
        if (s < 1) ++dies;
        myfile << I2P[j] << " " << dies << " " << I2S[s] << endl;
        ++s;
      }
    }
  }
  myfile.close();
}

// Funció que retorna un enter aleatori a l'interval [a, b]
int random(int a, int b) { 
  return a + rand() % (b-a+1); 
}

// Crea una col·loració aleatòria (un individu) sense sobreocupar les sales i utilitzant D colors com a màxim
vector<int> create_individual(int D) {
  vector<int> color(P, -1);
  vector<int> used(D, 0);
  int col = random(0, D-1);
  for (int p = 0; p < P; ++p) {
    col = random(0, D-1);
    while (used[col] >= S) col = random(0, D-1);
    ++used[col];
    color[p] = col;
  }
  M_used.push_back(used); 
  return color;
}

// Crida la funció create_individual per generar una població 
void create_population(vector<vector<int>>& population, int size, int D) {
  vector<int> individual;     // col·loració
  for (int i = 0; i < size; ++i) {
    individual = create_individual(D);
    population.push_back(individual);
  } 
}

/* 
Funció a MINIMITZAR. Consisteix en un parell <A,B> on 
  A = total d'incompatibilitats de pel·lícules
  B = nombre de dies emprats

MILLORA IMPLEMENTADA: --> "A més restriccions, més difícil és col·locar una peli":
  Posar malament una peli de "R" restriccions suma 2*R a A
*/
vector<pair<int,int>> fitness(const vector<vector<int>>& population, int D) {
  int population_size = population.size();
  vector<pair<int,int>> fitness(population_size);
  for (int i = 0; i < population_size; ++i) {
    for (int p = 0; p < P; ++p) {
      for(int r : G[p]) 
        if (population[i][p] == population[i][r]) 
          fitness[i].first = fitness[i].first + G[p].size();
    }
    if (fitness[i].first == 0) 
      fitness[i].second = D - count(M_used[i].begin(), M_used[i].end(), 0);
    else
      fitness[i].second = D;
  }
  return fitness;
}

// La mateixa idea de fitness per un sol individu
pair<int,int> ind_fitness(const vector<int>& individual, int D, int i) {
  pair<int,int> fitness;
  for (int p = 0; p < P; ++p) {
    for(int r : G[p]) 
      if (individual[p] == individual[r]) 
        fitness.first = fitness.first + G[p].size();
  }
  if (fitness.first == 0)
    fitness.second = D - count(M_used[i].begin(), M_used[i].end(), 0);
  else
    fitness.second = D;
  return fitness;
}

/*
MODIFICACIÓ DELS INDIVIDUS:
  Per a cada individu:      " millora els individus i si no, muta'ls"
  
  1) PERMUTACIÓ: Es fa un nombre "rand(1,3)" de permutacions de pelis entre dies i es comprova si la fitness millora. En cas contrari es repeteix el procés 100*P vegades

  2) Si s'arriba al màxim d'intents, MUTA l'individu: canvia aleatòriament una peli de dia
*/
void crossover (vector<vector<int>>& population, const vector<int>& parent, int D, int i) {

  vector<int> child(P);
  pair<int,int> fitness_p = ind_fitness(parent, D, i);
  pair<int,int> fitness_c; 

  int attempts = 0;
  
  int attempts_max =  100*P;
  bool trobat = false;
  while (!trobat && attempts <  attempts_max ) {
    int pers_max = random(1,3);
    int pers = 0;
    child = parent;
    while (pers < pers_max) {
      int pos1 = random(0, P-1);
      int pos2 = random(0, P-1);
      int temp = child[pos1];
      child[pos1] = child[pos2];
      child[pos2] = temp;
      pers++;
    }

    fitness_c = ind_fitness(child, D, i);
 
    if (fitness_c < fitness_p) trobat = true;
    attempts++;
  }

  // MUTACIÓ: només si no es troba cap permutació que millori l'individu
  if (!trobat) {
    child = parent;
    int position = random(0, P-1);
    int col = random(0, D-1);
    while (M_used[i][col] >= S or M_used[i][col] == 0) col = random(0, D-1);
    --M_used[i][child[position]];
    child[position] = col;
    ++M_used[i][col];
  }
  population.push_back(child);
}

/*
SELECCIÓ aleatòria d'individus per la nova població:
"com millor sigui l'individu (menor fitness) més probabilitat té de passar a la següent generació"

Poden haver diverses còpies d'un mateix individu si és prou bo
*/
vector<vector<int>> roulette_wheel (const vector<vector<int>>& population, int D) {
  vector<vector<int>> new_population;
  double tot_fit = 0;
  for(uint i = 0; i < population.size(); ++i) {
    pair<int,int> fit = ind_fitness(population[i], D, i);
    tot_fit += 1/(1+fit.first+fit.second);
  }
  vector<double> cum_fit;
  double cum_fit_sum = 0;
  for(uint i = 0; i < population.size(); ++i) {
    pair<int,int> fit = ind_fitness(population[i], D, i);
    cum_fit_sum += 1/(1+fit.first+fit.second)/tot_fit;
    cum_fit.push_back(cum_fit_sum);
  }
  for(uint i = 0; i < population.size(); ++i) {
    double roulette = ((double) rand() / (RAND_MAX)); // Uniform (0,1)
    int j = 0;
    while (roulette > cum_fit[j]) ++j;
    new_population.push_back(population[j]); 
    M_used[i] = M_used[j];
  }   
  return new_population;
}

/*
  CERCA DEL NOMBRE CROMÀTIC:
  
  Sabent que D_max=P i D_min=ceil(P/S), creem la població inicial amb un màxim D=P dies.
  Seguidament, intentem trobar una solució fent:
    1) Permutacions dels individus + mutació -> crossover
    2) Comprovem si dona lloc a un millor individu:
      Y: actualitza "best_fitness" o si és solució guarda-la a result.txt
      N: no_progress++ per reiniciar la població en cas d'estancar-se
    3) Selecció
  *Si generacions = 500 sense solució amb D o bé no_progress = 50, reinicia l'algoritme amb la última D.
  *Si es troba una solució, també es reinicia l'algoritme, però decrementant D en una unitat, fins arribar al mínim possible (D_min).
*/
void genetic_algorithm(int population_size) {
  int D = P;
  while (D >= ceil(P/S)) {
    int generation = 0;
    int no_progress = 0;

    vector<vector<int>> population;
    M_used.clear();   // Es global, i per tant, s'ha de buidar abans de crear una nova població
    create_population(population, population_size, D);

    pair<int,int> best_fitness (int(INT_MAX), D);
    vector<int> fittest_individual;

    // Escull el millor individu de la població generada aleatòriament
    vector<pair<int,int>> pop_fitness = fitness(population, D);
    for(int i = 0; i < population_size; ++i) {
      if (pop_fitness[i].first <= best_fitness.first) {
        if (pop_fitness[i].second <= best_fitness.second) {
          best_fitness = pop_fitness[i];
          fittest_individual = population[i];
        }
      }
    }

    // Busca una solució (best_fitness.first == 0) per la D actual o para en els casos descrits
    while (best_fitness.first != 0 and generation != 500 and no_progress != 50) {
      ++generation;
      ++no_progress;
    
      vector<vector<int>> new_population;
      for(int i = 0; i < population_size; ++i) 
        crossover(new_population, population[i], D, i);

      population = new_population;
      vector<pair<int,int>> pop_fitness = fitness(new_population, D);
      for(int i = 0; i < population_size; ++i) {
        if (pop_fitness[i].first <= best_fitness.first) {
          if (pop_fitness[i].second <= best_fitness.second) {
            // Si la fitness millora en restriccions i dies, hi ha progrès
            if (pop_fitness[i].first < best_fitness.first ) no_progress = 0;
            best_fitness = pop_fitness[i];
            fittest_individual = new_population[i];
          }
        }
      }
    }
    // Si s'ha trobat una solució, la guarda i fixa el seu valor de D com l'actual
    if (best_fitness.first == 0) {
      D = best_fitness.second;
      save(fittest_individual, D);
      --D;
    }
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
  
  // Crida l'algorisme genètic amb una població de 25 individus
  genetic_algorithm(25);
}