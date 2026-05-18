#include <iostream>
#include <cmath>
using namespace std;

// =====================================================================
//  N-Queens Problem - Genetic Algorithm
//  Compile: g++ -o nqueens nqueens_commented.cpp -std=c++11
//  Run:     ./nqueens
// =====================================================================

const int N          = 8;     // board size
const int POP_SIZE   = 200;   // number of solutions per generation
const int MAX_GEN    = 2000;  // maximum generations allowed
const int TOUR_K     = 5;     // tournament size
const double MUT_RATE = 0.05; // mutation probability per gene

// =====================================================================
//  Individual: one solution = array of 8 queen row positions
//  Example: [2,4,6,8,3,1,7,5] means:
//    col 0 -> queen at row 2
//    col 1 -> queen at row 4  ... etc
// =====================================================================
struct Individual {
    int genes[N];     // queen positions (rows 1..N)
    int fitness;      // how good this solution is
};

// =====================================================================
//  random helpers (no library needed)
// =====================================================================
int randomInt(int max) {
    return rand() % max;
}

double randomDouble() {
    return (double)rand() / RAND_MAX;
}

// =====================================================================
//  calculateFitness
//
//  Count pairs of queens that do NOT attack each other.
//  Max = N*(N-1)/2 = 28  -> perfect solution, no conflicts at all.
//  Two queens conflict if they share the same row or same diagonal.
// =====================================================================
int calculateFitness(int genes[N]) {
    int maxPairs = N * (N - 1) / 2;
    int conflicts = 0;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            // same row?
            if (genes[i] == genes[j]) {
                conflicts++;
                continue;
            }
            // same diagonal?
            if (abs(genes[i] - genes[j]) == abs(i - j)) {
                conflicts++;
            }
        }
    }
    return maxPairs - conflicts;
}

// =====================================================================
//  createRandom: fill an individual with random queen positions
// =====================================================================
Individual createRandom() {
    Individual ind;
    for (int i = 0; i < N; i++)
        ind.genes[i] = 1 + randomInt(N);
    ind.fitness = calculateFitness(ind.genes);
    return ind;
}

// =====================================================================
//  rouletteWheelSelection
//
//  Each individual gets a slice of the wheel equal to its fitness.
//  We spin the wheel (pick a random number) and return whoever we land on.
//  Higher fitness = bigger slice = higher chance of being picked.
// =====================================================================
Individual rouletteWheelSelection(Individual pop[POP_SIZE]) {
    // step 1: sum all fitness values
    int total = 0;
    for (int i = 0; i < POP_SIZE; i++)
        total += pop[i].fitness;

    if (total == 0)
        return pop[randomInt(POP_SIZE)];

    // step 2: spin the wheel
    int point = randomInt(total + 1);

    // step 3: walk through until we reach the point
    int cumulative = 0;
    for (int i = 0; i < POP_SIZE; i++) {
        cumulative += pop[i].fitness;
        if (cumulative >= point)
            return pop[i];
    }
    return pop[POP_SIZE - 1];
}

// =====================================================================
//  tournamentSelection
//
//  Pick TOUR_K random individuals, return the one with highest fitness.
//  Like a mini-competition: best of k random players wins.
// =====================================================================
Individual tournamentSelection(Individual pop[POP_SIZE]) {
    Individual best = pop[randomInt(POP_SIZE)];

    for (int i = 1; i < TOUR_K; i++) {
        Individual candidate = pop[randomInt(POP_SIZE)];
        if (candidate.fitness > best.fitness)
            best = candidate;
    }
    return best;
}

// =====================================================================
//  crossover
//
//  Pick a random cut point, take left part from parent1
//  and right part from parent2 to make one child.
//  Example (cut=3): [1,2,3|4,5,6,7,8] + [8,7,6|5,4,3,2,1]
//                -> child = [1,2,3,5,4,3,2,1]
// =====================================================================
Individual crossover(Individual parent1, Individual parent2) {
    Individual child;
    int cutPoint = 1 + randomInt(N - 1);

    for (int i = 0; i < cutPoint; i++)
        child.genes[i] = parent1.genes[i];
    for (int i = cutPoint; i < N; i++)
        child.genes[i] = parent2.genes[i];

    child.fitness = calculateFitness(child.genes);
    return child;
}

// =====================================================================
//  mutate
//
//  For each gene: with probability MUT_RATE, replace it with a
//  random value. Prevents the algorithm from getting stuck.
// =====================================================================
void mutate(Individual &ind) {
    for (int i = 0; i < N; i++) {
        if (randomDouble() < MUT_RATE) {
            ind.genes[i] = 1 + randomInt(N);
        }
    }
    ind.fitness = calculateFitness(ind.genes);
}

// =====================================================================
//  findBestIndex: return index of highest fitness individual
// =====================================================================
int findBestIndex(Individual pop[POP_SIZE]) {
    int best = 0;
    for (int i = 1; i < POP_SIZE; i++)
        if (pop[i].fitness > pop[best].fitness)
            best = i;
    return best;
}

// =====================================================================
//  printBoard: display the chessboard
// =====================================================================
void printBoard(int genes[N]) {
    for (int row = 1; row <= N; row++) {
        cout << "  |";
        for (int col = 0; col < N; col++)
            cout << (genes[col] == row ? " Q |" : " . |");
        cout << "\n";
    }
    for (int i = 0; i < N * 4 + 3; i++) cout << "-";
    cout << "\n";
}

// =====================================================================
//  runGA: main Genetic Algorithm
//
//  1. Build initial population (seed + random individuals)
//  2. Evaluate fitness for everyone
//  3. Loop:
//       - Keep best individual (elitism)
//       - Select parents -> crossover -> mutate -> add to new generation
//       - Stop when fitness == 28 (perfect) or max generations reached
// =====================================================================
void runGA(int initialState[N], int caseNum) {
    srand((unsigned int)time(0) + caseNum * 1000);

    int target = N * (N - 1) / 2; // = 28

    // build population
    Individual pop[POP_SIZE];
    for (int i = 0; i < N; i++)
        pop[0].genes[i] = initialState[i];
    pop[0].fitness = calculateFitness(pop[0].genes);
    for (int i = 1; i < POP_SIZE; i++)
        pop[i] = createRandom();

    Individual bestEver = pop[findBestIndex(pop)];
    int fitnessHistory[MAX_GEN];
    int generationsUsed = MAX_GEN;

    clock_t startTime = clock();

    // main loop
    for (int gen = 0; gen < MAX_GEN; gen++) {
        int bestIdx = findBestIndex(pop);
        if (pop[bestIdx].fitness > bestEver.fitness)
            bestEver = pop[bestIdx];

        fitnessHistory[gen] = bestEver.fitness;

        if (bestEver.fitness == target) {
            generationsUsed = gen + 1;
            break;
        }

        // build next generation
        Individual newPop[POP_SIZE];
        newPop[0] = pop[bestIdx]; // elitism: keep best unchanged

        for (int i = 1; i < POP_SIZE; i++) {
            Individual p1    = rouletteWheelSelection(pop);
            Individual p2    = tournamentSelection(pop);
            Individual child = crossover(p1, p2);
            mutate(child);
            newPop[i] = child;
        }

        for (int i = 0; i < POP_SIZE; i++)
            pop[i] = newPop[i];
    }

    double timeElapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;

    // print results
    cout << "\n##################################################\n";
    cout << "#  Test Case " << caseNum << "\n";
    cout << "##################################################\n";

    cout << "Initial state : [";
    for (int i = 0; i < N; i++) {
        cout << initialState[i];
        if (i < N - 1) cout << ",";
    }
    cout << "]\n";

    cout << "Status        : " << (bestEver.fitness == target ? "SOLVED" : "NOT SOLVED") << "\n";
    cout << "Best fitness  : " << bestEver.fitness << " / " << target << "\n";
    cout << "Generations   : " << generationsUsed << "\n";
    cout << "Time          : " << timeElapsed << " seconds\n";

    cout << "Best solution : [";
    for (int i = 0; i < N; i++) {
        cout << bestEver.genes[i];
        if (i < N - 1) cout << ",";
    }
    cout << "]\n\n";

    cout << "Board:\n";
    printBoard(bestEver.genes);

    cout << "\nFitness progress (first 10 generations):\n";
    int limit = (generationsUsed < 10) ? generationsUsed : 10;
    for (int i = 0; i < limit; i++)
        cout << "  Gen " << (i + 1) << "  ->  fitness = " << fitnessHistory[i] << "\n";
    if (generationsUsed > 10) {
        cout << "  ...\n";
        cout << "  Gen " << generationsUsed << "  ->  fitness = " << fitnessHistory[generationsUsed - 1] << "\n";
    }
}

// =====================================================================
//  main
// =====================================================================
int main() {
    cout << "==================================================\n";
    cout << "   N-Queens Solver using Genetic Algorithm\n";
    cout << "==================================================\n";

   int cases[5][N];

for (int i = 0; i < 5; i++) {
     cout << "Enter case " << i + 1 << ":\n";

    for (int j = 0; j < N; j++) {
        cin >> cases[i][j];
    }
};

    for (int i = 0; i < 5; i++)
        runGA(cases[i], i + 1);

    cout << "\n==================================================\n";
    cout << "Done!\n";
    cout << "==================================================\n";

    return 0;
}
