#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
#pragma GCC target("avx")  //Enable AVX

#include<bits/stdc++.h>

#define ull unsigned long long
#define pb push_back
#define u64 uint64_t
#define INLINE inline

using namespace std;

const int MAX_N = 200;

const int MAX_EDGE = 100000;

auto start_time = chrono::high_resolution_clock::now();

int TimeForTurn = 980;


enum resource : int8_t {
    kEmpty = 0,
    kCrystal = 1,
    kEggs = 2
};
string translate(resource resource_)
{
    switch(resource_)
    {
        case kEmpty :
            return "Empty";
        case kCrystal :
            return "Crystal";
        case kEggs :
            return "Eggs";
    };
    return "NONE";
}


template <typename T, u64 MaxSize>
class Buffer {
 public:
  Buffer() : size_(0) {}

  INLINE void Reset() { size_ = 0; }

  INLINE void Add(const T& val) { buffer_[size_++] = val; }

  INLINE const T& operator[](u64 idx) const { return buffer_[idx]; }

  INLINE void PopBack() { --size_; }

  INLINE T& operator[](u64 idx) { return buffer_[idx]; }

  INLINE u64 Size() const { return size_; }

  INLINE void SetSize(u64 size) { size_ = size; }

//   INLINE void RandomShuffle(Random& rnd) { ::RandomShuffle(buffer_, buffer_ + size_, rnd); }

//   INLINE const T& Random(Random& rnd) const { return buffer_[rnd.NextInt(size_)]; }

  auto begin() const { return buffer_; }

  auto end() const { return buffer_ + size_; }

  auto begin() { return buffer_; }

  auto end() { return buffer_ + size_; }

 private:
  T buffer_[MaxSize];
  u64 size_;
};

struct cell
{
    void Start(resource resourceType_, int resourceCount_, int neigbour_)
    {
        if(neigbour_ != -1)
        {
            neighbours.Add(neigbour_);
        }
        resourceType = resourceType_;
        resourceCount = resourceCount_;
    }
    void Update(int newResourceCount_, int myAnts_, int enemyAnts_)
    {
        resourceCount = newResourceCount_;
        enemyAnts = enemyAnts_;
        myAnts = myAnts_;
        chainEnemy = 0;
        chainMe = 0;
    }
    void placeBeacon(int beaconVal_)
    {
        beacon = beaconVal_;
    }

    void resetChain()
    {
        chainEnemy = 0;
        chainMe = 0;
    }
    Buffer<int,6> neighbours;
    int enemyAnts;
    int myAnts;
    resource resourceType = kEmpty;
    int beacon = 0;
    int resourceCount = 0;
    int chainEnemy = 0;
    int chainMe = 0;
    int wiggleRoom = 0;
};

double rand_DoubleRange(double a, double b)
{
    return ((b-a) * ((double)rand() / RAND_MAX)) + a;
}



cell cells[MAX_N];
cell cellsCopy[MAX_N];
cell cellsCopy2[MAX_N];

Buffer<int,2> myBase, enemyBase;
Buffer<double,MAX_EDGE> weights;

Buffer<int, MAX_EDGE> beaconCells;
Buffer<int, MAX_EDGE> antCells;

Buffer<pair<int,pair<int,int>>, MAX_EDGE> allPairs;
Buffer<pair<int,pair<int,int>>, MAX_EDGE> allPairsCopy;
Buffer<pair<int,pair<int,int>>, MAX_EDGE> allocations;


Buffer<pair<int,pair<int,int>>, MAX_EDGE> moves;




Buffer<pair<int,int>,MAX_EDGE> alreadyChoosed;
Buffer<pair<int,int>,MAX_EDGE> candidatesToPick; 


Buffer<pair<int,int>,MAX_EDGE> bestPath; 


int shortestDist[MAX_N][MAX_N]; // From A to B

bool alreadyInPath[MAX_N];

int numberOfCells; 

int allMyAnts = 0;
int allCrystal = 0;

bool earlyGame = 0;



void generateShortestPath()
{
    for (int cell = 0; cell < numberOfCells; cell++)
    {
        queue<int> kol;
        kol.push(cell);
        shortestDist[cell][cell] = 1;

        while(!kol.empty())
        {
            auto top = kol.front(); kol.pop();
            
            for (auto neigbour: cells[top].neighbours)
            {
                if(shortestDist[cell][neigbour] == 0)
                {
                    shortestDist[cell][neigbour] = shortestDist[cell][top] + 1;
                    kol.push(neigbour);
                }
            }
        }
    }
    cerr << "shortestPathFound!\n";
}



void generateCandidates()
{
    candidatesToPick.Reset();

    for(int i = 0 ; i < numberOfCells; i++)
        alreadyInPath[i]  = 0;
    
    for(auto alreadyPicked : alreadyChoosed)
    {
        alreadyInPath[alreadyPicked.first] = 1;
        alreadyInPath[alreadyPicked.second] = 1;
    }
        

    for(auto alreadyPicked : alreadyChoosed)
    {
        for(int cell = 0 ; cell < numberOfCells; cell++)
        {
            if(cells[cell].resourceType != kEmpty && cells[cell].resourceCount != 0 && alreadyInPath[cell] == 0) candidatesToPick.Add({alreadyPicked.second, cell});
        }
    }
}

double generateWeight(int from, int to)
{
    assert(shortestDist[from][to] != 0);
    return (double)(0.5 / (double) (shortestDist[from][to]));
}

pair<int,int> getScore()
{
    pair<int,int> capturedResources; // first Crystal, second Eggs

    for(int i = 0; i < numberOfCells; i++)
        cells[i].resetChain();

    

    priority_queue<pair<int,int>> kol;

    for(auto base : enemyBase)
    {
        kol.push({cells[base].enemyAnts,base});
        cells[base].chainEnemy = cells[base].enemyAnts;
    }

    while(!kol.empty())
    {
        auto top = kol.top(); kol.pop();
        
        for (auto neigbour: cells[top.second].neighbours)
        {
            int newValue = min(cells[top.second].chainEnemy, cells[neigbour].enemyAnts);
            
            if(newValue != 0 && cells[neigbour].chainEnemy < newValue)
            {
                cells[neigbour].chainEnemy = newValue;
                kol.push({cells[neigbour].chainEnemy,neigbour});
            }
                
        }
    }

    for(auto base : myBase)
    {
        kol.push({cells[base].myAnts,base});
        cells[base].chainMe = cells[base].myAnts;
    }

    while(!kol.empty())
    {
        auto top = kol.top(); kol.pop();
                
        if(cells[top.second].resourceType != kEmpty)
        {
            if(cells[top.second].resourceType == kCrystal)
            {
                capturedResources.first += min(cells[top.second].chainMe, cells[top.second].resourceCount);
                cells[top.second].resourceCount -= min(cells[top.second].chainMe, cells[top.second].resourceCount);
            } else
            {
                capturedResources.second += min(cells[top.second].chainMe, cells[top.second].resourceCount);
                cells[top.second].resourceCount -= min(cells[top.second].chainMe, cells[top.second].resourceCount);
            }
        }

        for (auto neigbour: cells[top.second].neighbours)
        {

            int newValue = min(cells[top.second].chainMe, cells[neigbour].myAnts);
            
            if(newValue != 0 && cells[neigbour].chainMe < newValue && newValue >= cells[neigbour].chainEnemy)
            {
                cells[neigbour].chainMe = newValue;
                kol.push({cells[neigbour].chainMe,neigbour});
            }
        }
    }


    return capturedResources;



}

void placeBeacons()
{
    for(int i = 0 ; i < numberOfCells; i++)
        cells[i].placeBeacon(0);
    
    for(auto edge : alreadyChoosed)
    {  
        int startPoint = edge.first, endPoint = edge.second;

        cells[startPoint].placeBeacon(1);
        while(endPoint != startPoint)
        {
            cells[endPoint].placeBeacon(1);
            for(auto neigbour : cells[endPoint].neighbours)
            {
                if(shortestDist[startPoint][endPoint] == shortestDist[startPoint][neigbour] + 1)
                {
                    endPoint = neigbour;
                    break;
                }
            }
        }
        
    }

}

void placeBeaconsCout()
{
    for(int i = 0 ; i < numberOfCells; i++)
        cells[i].placeBeacon(0);
    
    for(auto edge : bestPath)
    {  
        int startPoint = edge.first, endPoint = edge.second;

        cells[startPoint].placeBeacon(1);
        while(endPoint != startPoint)
        {
            cells[endPoint].placeBeacon(1);
            for(auto neigbour : cells[endPoint].neighbours)
            {
                if(shortestDist[startPoint][endPoint] == shortestDist[startPoint][neigbour] + 1)
                {
                    endPoint = neigbour;
                    break;
                }
            }
        }
        
    }

}

int evalBoard(int turns)
{   
    pair<int,int> collectedResources;
    for(int i = 0; i < numberOfCells; i++)
    {
        cellsCopy[i] = cells[i];
    }



    placeBeacons();
    

    for(int turn = 0 ; turn < turns; turn++)
    {
        for(int i = 0 ; i < numberOfCells; i++)
        {
            cellsCopy2[i] = cells[i];
        }
        allocations.Reset();
        beaconCells.Reset();
        antCells.Reset();
        allPairs.Reset();
        moves.Reset();


        int antSum = 0;
        int beaconSum = 0;

        for(int i = 0; i < numberOfCells; i++)
        {
            if(cells[i].beacon > 0)
            {
                beaconCells.Add(i);
                beaconSum += cells[i].beacon;
            } 
            if(cells[i].myAnts > 0)
            {
                antCells.Add(i);
                antSum += cells[i].myAnts;
            } 
        }

        double scalingFactor = (double) antSum / beaconSum;

        for(auto beaconCell : beaconCells)
        {
            int highBeaconValue = (int) ceil(cells[beaconCell].beacon * scalingFactor);
            int lowBeaconValue = (int) (cells[beaconCell].beacon * scalingFactor);
            cells[beaconCell].beacon = max(1,lowBeaconValue);
            cells[beaconCell].wiggleRoom = highBeaconValue - lowBeaconValue;
        }


        // cerr << "BEFORE\n";
        // for(int i = 0 ; i < numberOfCells; i++)
        // {
        //     cerr << i << " myAnts: " << cells[i].myAnts << " Beacon: " << cells[i].beacon << "\n";
        // }



        
        for(auto antCell : antCells)
        {
            for(auto beaconCell : beaconCells)
            {
                allPairs.Add({shortestDist[antCell][beaconCell],{antCell, beaconCell}});
            }
        }

        sort(allPairs.begin(), allPairs.end());

        // for(auto pair : allPairs)
        // {
        //     cerr << "============= " << pair.first << " " << pair.second.first << " " << pair.second.second << "\n";
        // }

        bool stragglers = 0;
        while(allPairs.Size() != 0)
        {
            for(auto pair : allPairs)
            {
                int antCell = pair.second.first;
                int beaconCell = pair.second.second;

                int antCount = cells[antCell].myAnts;

                int beaconCount = cells[beaconCell].beacon;
                int wiggleRoom = cells[beaconCell].wiggleRoom;

                int maxAlloc = (int) (stragglers ? min(antCount, beaconCount + wiggleRoom) : min(antCount, beaconCount));
                if(maxAlloc > 0)
                {
                    allocations.Add({maxAlloc, {antCell, beaconCell}});
                    cells[antCell].myAnts -= maxAlloc;

                    if(!stragglers)
                    {
                        cells[beaconCell].beacon -= maxAlloc;
                    } else 
                    {
                        cells[beaconCell].beacon -= (maxAlloc - wiggleRoom);
                        cells[beaconCell].wiggleRoom = 0;
                    }
                }
            }
            allPairsCopy.Reset();
            for(auto it : allPairs)
            {
                if(cells[it.second.first].myAnts > 0)
                    allPairsCopy.Add(it);
            }
            allPairs.Reset();
            for(auto it : allPairsCopy)
            {
                allPairs.Add(it);
            }


            stragglers = true;
        }

        for(int i = 0 ; i < numberOfCells; i++)
        {
            cells[i] = cellsCopy2[i];
        }

        for(auto alloc : allocations)
        {   
            
            int from = alloc.second.first, to = alloc.second.second, amount = alloc.first;

            //cerr << "============ " << from << " " << to << " " << amount << "\n";

            if(from == to) continue;

            int bestNeigbour = -1;
            for(auto neigbour: cells[from].neighbours)
            {
                if(shortestDist[to][from] == shortestDist[to][neigbour] + 1)
                {
                    if (bestNeigbour == -1)
                        bestNeigbour = neigbour;
                     
                    if(cells[neigbour].myAnts > cells[bestNeigbour].myAnts)
                    {
                        bestNeigbour = neigbour;
                    } else if (cells[neigbour].myAnts == cells[bestNeigbour].myAnts)
                    {
                        if(cells[neigbour].beacon > cells[bestNeigbour].beacon)
                        {
                            bestNeigbour = neigbour;
                        } else if(cells[neigbour].beacon == cells[bestNeigbour].beacon)
                        {
                            if(neigbour > bestNeigbour)
                            {
                                bestNeigbour = neigbour;
                            }
                        }
                    }
                }
            }
            moves.Add({amount,{from, bestNeigbour}});
        }

        for(auto move : moves)
        {
            int from = move.second.first;
            int to = move.second.second;

            int amount = move.first;

            //cerr << "MOVE: " << from << " " << to << " " << amount << "\n";

            cells[from].myAnts -= amount;
            cells[to].myAnts += amount;
        }

        // cerr << "AFTER\n";
        // for(int i = 0 ; i < numberOfCells; i++)
        // {
        //     cerr << i << " myAnts: " << cells[i].myAnts << " Beacon: " << cells[i].beacon << "\n";
        // }


        pair<int,int> afterTurnScore = getScore();
        collectedResources.first += afterTurnScore.first;
        collectedResources.second += afterTurnScore.second;



    }

    for(int i = 0; i < numberOfCells; i++)
    {
        cells[i] = cellsCopy[i];
    }

    double weight = 2;
    if(earlyGame) weight = 6;

    int collectedEval = collectedResources.first + collectedResources.second * weight;

    int furthestEval = 0;
    for(auto base: myBase)
    {
        // for(int i = 0 ; i < numberOfCells; i++)
        // {
        //     furthestEval = max(furthestEval, shortestDist[base][i]);
        // }
    }

    return collectedEval + 100*furthestEval;
}
int simulations = 0;
void generatePath()
{
    start_time = chrono::high_resolution_clock::now();
    int bestEval = 0;

    allMyAnts = 0;
    for(int i = 0 ; i < numberOfCells; i++)     // licze ile mam mrowek
        allMyAnts += cells[i].myAnts;

    simulations = 0;


    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count() <= TimeForTurn)
    {
        //if(simulations > 10) break;

        simulations ++;
        alreadyChoosed.Reset(); // na poczatku nie wybralem nic

        for (auto base : myBase) 
        {
            alreadyChoosed.Add({base,base});   // potem dodaje bazy (je na pewno mam)
        }


        
        int pathCost = 0;
        //cerr << "generating path beep boop\n";

        while(alreadyChoosed.Size() < 8)
        {

            generateCandidates();        // generowanie mozliwych krawedzi


            // cerr << "allCandidatesGenerated!\n";
            // cerr << "current time: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count() << "\n";
            // cerr << "size of currentPath " << alreadyChoosed.Size() << "\n";

            if(candidatesToPick.Size() == 0) break; // jak nie ma to eloo

            weights.Reset();   // ppb dla kazdej krawedzi

            for(auto candidate : candidatesToPick)
            {
                weights.Add(generateWeight(candidate.first,candidate.second));  // tworze ppb z kandydatow
            }

            double sum = 0;
            for(auto weight : weights) // suma ppb
                sum += weight;  

            double rand = rand_DoubleRange(0.0,sum); // wybieram krawedz

            sum = 0;
            for(int i = 0; i < weights.Size(); i++)
            {
                sum += weights[i];      // szukanie krawedzi jaka wylosowalem
                if(sum > rand) 
                {
                    pathCost += shortestDist[candidatesToPick[i].first][candidatesToPick[i].second];
                    if(pathCost <= allMyAnts)   // jesli moge ja miec to dodaj do choosed
                        alreadyChoosed.Add({candidatesToPick[i].first,candidatesToPick[i].second});
                    break;
                }
            }

            if(pathCost > allMyAnts) break;
            // cerr << "picked edges:\n";
            // for(auto edge : alreadyChoosed)
            // {
            //     cerr << edge.first << " " << edge.second << "\n";
            // }


            int currEval = evalBoard(4);
            //cerr << "E: " << currEval << "\n";
            if(bestEval < currEval)
            {
                cerr << "E: " << currEval << "\n";
                bestEval = currEval;
                bestPath.Reset();
                for(auto edge : alreadyChoosed)
                    bestPath.Add(edge);
            }    
        }
        //cerr << "size of currentPath " << alreadyChoosed.Size() << "\n";
    }

    cerr << "S: " << simulations << "\n";

    cerr << "path generated boop\n";
}

void prepare()
{
    cin >> numberOfCells;

    for(int i = 0 ; i < numberOfCells; i++)
    {
        int type, resourceCount;  cin >> type >> resourceCount; 
        for(int j = 0; j < 6; j++)
        {
            int neighbour; cin >> neighbour;
            resource resourceType = type == 0 ? kEmpty : (type == 1 ? kEggs : kCrystal);

            cells[i].Start(resourceType, resourceCount, neighbour);
        }

        if(cells[i].resourceType == kCrystal) allCrystal += resourceCount;
    }

    int numberOfBases; cin >> numberOfBases;
    for(int i = 0 ; i < numberOfBases; i++)
    {
        int baseID; cin >> baseID;
        myBase.Add(baseID);
    }
    for(int i = 0 ; i < numberOfBases; i++)
    {
        int baseID; cin >> baseID;
        enemyBase.Add(baseID);
    }

    generateShortestPath();

}

int main()
{
    srand(time(NULL));

    prepare();

    while(true)
    {

        int currentCrystalCount = 0;

        int myScore, enemyScore; cin >> myScore >> enemyScore;


        for(int i = 0; i < numberOfCells; i++)
        {
            int resourcesCount, myAnts, enemyAnts; cin >> resourcesCount >> myAnts >> enemyAnts;
            cells[i].Update(resourcesCount, myAnts, enemyAnts);
            if (cells[i].resourceType == kCrystal) currentCrystalCount += resourcesCount;
        }
        earlyGame = (100.0 * currentCrystalCount / allCrystal) > 85;

        cerr << "early game : " << earlyGame << "\n";

        generatePath();


        if(bestPath.Size() == 0)
        {
            cout << "WAIT\n";
        }


        placeBeaconsCout();



        for(auto edge : bestPath)
        {
            if(edge.first != edge.second)
                cerr << edge.first << " " << edge.second << " ";
        }
        cerr << "\n";

        for(int i = 0 ; i < numberOfCells; i++)
        {
            if(cells[i].beacon > 0)
            {
                cout << "BEACON " << i << " " << cells[i].beacon << ";";
            }
        }
        cout << "MESSAGE T: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count() << " S: " << simulations;
        cout << endl;

        cerr << "TO BY BYLO NA TYLE\n";

        TimeForTurn = 95;
        //return -1;
    }


}