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

auto startTime = chrono::high_resolution_clock::now();

int timeForChoosing = 500;
int timeForUpgrading = 380;

int maxEdgeCount = 8;


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

struct test
{
    int8_t amount = 0;
    int8_t from = 0;
    int8_t to = 0;
    test(){}
    test(int8_t amount_, int8_t from_, int8_t to_)
    {
        amount = amount_;
        from = from_;
        to = to_;
    }
    void set(int8_t amount_, int8_t from_, int8_t to_)
    {
        amount = amount_;
        from = from_;
        to = to_;
    }
};


cell cells[MAX_N];
cell cellsCopy[MAX_N];
cell cellsCopy2[MAX_N];
cell cellsCopy3[MAX_N];

Buffer<int,2> myBase, enemyBase;
Buffer<double,MAX_EDGE> weights;

Buffer<int, MAX_EDGE> beaconCells;
Buffer<int, MAX_EDGE> antCells;

Buffer<test, MAX_EDGE> allPairs;
Buffer<test, MAX_EDGE> allPairsCopy;
Buffer<test, MAX_EDGE> allocations;


Buffer<test, MAX_EDGE> moves;

Buffer<pair<int,int>,MAX_EDGE> sortedPairs[MAX_N];


Buffer<pair<int,int>,MAX_EDGE> alreadyChoosed;
Buffer<pair<int,int>,MAX_EDGE> candidatesToPick; 


Buffer<pair<int,int>,MAX_EDGE> bestPath; 

Buffer<int,MAX_N> queueForDial[2000];


int shortestDist[MAX_N][MAX_N]; // From A to B

bool alreadyInPath[MAX_N];

int numberOfCells; 

int allMyAnts = 0;
int allCrystal = 0;

bool earlyGame = 0;

int simulations = 0;

int upgradeSimulations = 0;


queue<int> kol;

void generateShortestPath()
{
    
    for (int cell = 0; cell < numberOfCells; cell++)
    {
        
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
}


double generateWeight(int from, int to)
{
    assert(shortestDist[from][to] != 0);
    int enemyDist = 1e9;
    for(auto base : enemyBase)
        enemyDist = min(enemyDist, shortestDist[to][base]);

    int myDist = 1e9;
    for(auto base : myBase)
        myDist = min(myDist, shortestDist[to][base]);

    int diff = abs(myDist - enemyDist) + 1;

    
    // double valOfResource = cells[to].resourceType == kCrystal ? 1.0 - (cells[to].resourceCount/allCrystal) :;

    return (double)(0.5 / (double) (diff));
}

pair<int,int> getScore()
{
    pair<int,int> capturedResources; // first Crystal, second Eggs

    for(int i = 0; i < numberOfCells; i++)
        cells[i].resetChain();

    
    //priority_queue<pair<int,int>> kol; // TODO DIAL ALGORITHM

    
    int idx = 0;

    for(auto base : enemyBase)
        idx = max(idx,cells[base].enemyAnts);

    for(int i = 0; i <= idx; i++)
        queueForDial[i].Reset();

    for(auto base : enemyBase)
    {
        queueForDial[cells[base].enemyAnts].Add(base);
        cells[base].chainEnemy = cells[base].enemyAnts;
    }

    

    assert(idx < 2000);


    while(idx != 0)
    {
        //cerr << idx << " " << queueForDial[idx].Size() << "\n";
        while(queueForDial[idx].Size() == 0 && idx != 0) idx --;

        if(idx == 0 || queueForDial[idx].Size() == 0) break;

        auto top = queueForDial[idx][queueForDial[idx].Size() - 1]; queueForDial[idx].PopBack();

        for (auto neigbour: cells[top].neighbours)
        {
            int newValue = min(cells[top].chainEnemy, cells[neigbour].enemyAnts);
            
            if(newValue != 0 && cells[neigbour].chainEnemy < newValue)
            {
                cells[neigbour].chainEnemy = newValue;
                queueForDial[cells[neigbour].chainEnemy].Add(neigbour);
            }
        }
    }

    // for(int i = 0; i < 2000; i++)
    //     queueForDial[i].Reset();

    for(auto base : myBase)
        idx = max(idx,cells[base].myAnts);

    for(int i = 0; i <= idx; i++)
        queueForDial[i].Reset();

    for(auto base : myBase)
    {
        queueForDial[cells[base].myAnts].Add(base);
        cells[base].chainMe = cells[base].myAnts;
    }


    assert(idx < 2000);

    while(idx != 0)
    {
        while(queueForDial[idx].Size() == 0 && idx != 0) idx --;

        if(idx == 0 || queueForDial[idx].Size() == 0) break;

        auto top = queueForDial[idx][queueForDial[idx].Size() - 1]; queueForDial[idx].PopBack();

        if(cells[top].resourceType == kEggs)
        {
            capturedResources.second += min(cells[top].chainMe, cells[top].resourceCount);
            cells[top].resourceCount -= min(cells[top].chainMe, cells[top].resourceCount);
        }
        

        for (auto neigbour: cells[top].neighbours)
        {

            int newValue = min(cells[top].chainMe, cells[neigbour].myAnts);
            
            if(newValue != 0 && cells[neigbour].chainMe < newValue && newValue >= cells[neigbour].chainEnemy)
            {
                cells[neigbour].chainMe = newValue;
                queueForDial[cells[neigbour].chainMe].Add(neigbour);
            }
        }
    }


    for(auto base : myBase)
        idx = max(idx,cells[base].myAnts + capturedResources.second);

    for(int i = 0; i <= idx; i++)
        queueForDial[i].Reset();

    for(int i = 0; i < numberOfCells; i++)
        cells[i].chainMe = 0;

    for(auto base : myBase)
    {
        queueForDial[cells[base].myAnts + capturedResources.second].Add(base);
        cells[base].chainMe = cells[base].myAnts + capturedResources.second;
    }


    while(idx != 0)
    {
        while(queueForDial[idx].Size() == 0 && idx != 0) idx --;

        if(idx == 0 || queueForDial[idx].Size() == 0) break;

        auto top = queueForDial[idx][queueForDial[idx].Size() - 1]; queueForDial[idx].PopBack();

        
        if(cells[top].resourceType == kCrystal)
        {
            double weightCrytal = 5.0;

            int enemyDist = 1e9;
            for(auto base : enemyBase)
                enemyDist = min(enemyDist, shortestDist[top][base]);

            int myDist = 1e9;
            for(auto base : myBase)
                myDist = min(myDist, shortestDist[top][base]);

            
            weightCrytal *= 1.0 / (double) (abs(enemyDist - myDist) + 1);

            weightCrytal = max(1.0, weightCrytal);

            int extracted = min(cells[top].chainMe, cells[top].resourceCount) * weightCrytal;

            capturedResources.first += extracted;
            cells[top].resourceCount -= min(cells[top].chainMe, cells[top].resourceCount);
        }
        for (auto neigbour: cells[top].neighbours)
        {

            int newValue = min(cells[top].chainMe, cells[neigbour].myAnts);
            
            if(newValue != 0 && cells[neigbour].chainMe < newValue && newValue >= cells[neigbour].chainEnemy)
            {
                cells[neigbour].chainMe = newValue;
                queueForDial[cells[neigbour].chainMe].Add(neigbour);
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

        if(startPoint != endPoint) cells[startPoint].placeBeacon(1);
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



        int id = 0;
        for(auto antCell : antCells)
        {
            for(auto beaconCell : beaconCells)
            {
                allPairs[id++].set(shortestDist[antCell][beaconCell],antCell, beaconCell);
            }
        }
        allPairs.SetSize(id);

        for(int i = 0 ; i < numberOfCells; i++)
        {
            sortedPairs[i].Reset();
        }

        for(auto pair : allPairs)
        {
            sortedPairs[pair.amount].Add({pair.from, pair.to});
        }   

        id = 0;
        for(int i = 0 ; i < numberOfCells; i++)
        {
            for(auto pair : sortedPairs[i])
            {
                allPairs[id++].set(i,pair.first, pair.second);
            }
        }

        allPairs.SetSize(id);

        bool stragglers = 0;
        id = 0;
        while(allPairs.Size() != 0)
        {
            for(auto pair : allPairs)
            {
                int antCell = pair.from;
                int beaconCell = pair.to;

                int antCount = cells[antCell].myAnts;

                int beaconCount = cells[beaconCell].beacon;
                int wiggleRoom = cells[beaconCell].wiggleRoom;

                int maxAlloc = (int) (stragglers ? min(antCount, beaconCount + wiggleRoom) : min(antCount, beaconCount));
                if(maxAlloc > 0)
                {
                    allocations[id++].set(maxAlloc, antCell, beaconCell);
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
                if(cells[it.from].myAnts > 0)
                    allPairsCopy.Add(it);
            }
            allPairs.Reset();
            for(auto it : allPairsCopy)
            {
                allPairs.Add(it);
            }


            stragglers = true;
        }

        allocations.SetSize(id);

        for(int i = 0 ; i < numberOfCells; i++)
        {
            cells[i] = cellsCopy2[i];
        }

        id = 0;
        for(auto alloc : allocations)
        {   
            
            int from = alloc.from, to = alloc.to, amount = alloc.amount;

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
            moves[id++].set(amount,from, bestNeigbour);
        }
        moves.SetSize(id);

        for(auto move : moves)
        {
            int from = move.from;
            int to = move.to;
            int amount = move.amount;
            cells[from].myAnts -= amount;
            cells[to].myAnts += amount;
        }

        pair<int,int> afterTurnScore = getScore();
        collectedResources.first += afterTurnScore.first;
        collectedResources.second += afterTurnScore.second;



    }

    for(int i = 0; i < numberOfCells; i++)
    {
        cells[i] = cellsCopy[i];
    }

    double weight = myBase.Size() == 2 ? 4 : 2;
    if(earlyGame) weight = myBase.Size() == 2 ? 10 : 5;
    
    int collectedEval = collectedResources.first + collectedResources.second * weight;

    return collectedEval;
}

void generatePath()
{
    startTime = chrono::high_resolution_clock::now();
    int bestEval = -1;
    int bestEvalPathLength = 0;

    allMyAnts = 0;
    for(int i = 0 ; i < numberOfCells; i++)     // licze ile mam mrowek
        allMyAnts += cells[i].myAnts;

    simulations = 0;


    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count() <= timeForChoosing)
    {
        //if(simulations > 10) break;

        simulations ++;
        alreadyChoosed.Reset(); // na poczatku nie wybralem nic
        candidatesToPick.Reset();

        for(int cell = 0; cell < numberOfCells; cell++) // reset of bool table
            alreadyInPath[cell] = 0;

        for (auto base : myBase) 
        {
            alreadyChoosed.Add({base,base});   // potem dodaje bazy (je na pewno mam)
            alreadyInPath[base] = 1;

        }

        for(auto node : alreadyChoosed)
        {
            for(int cell = 0 ; cell < numberOfCells; cell++)
            {
                if(cells[cell].resourceType != kEmpty && cells[cell].resourceCount != 0 && alreadyInPath[cell] == 0) candidatesToPick.Add({node.first, cell});
            }
        }

        
        int pathCost = 0;
        //cerr << "generating path beep boop\n";

        while(alreadyChoosed.Size() < (myBase.Size() == 2 ? 10 : maxEdgeCount))
        {

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
            int nodeAdded = -1;
            for(int i = 0; i < weights.Size(); i++)
            {
                sum += weights[i];      // szukanie krawedzi jaka wylosowalem
                if(sum > rand) 
                {
                    pathCost += shortestDist[candidatesToPick[i].first][candidatesToPick[i].second];
                    if(pathCost <= allMyAnts)   // if I can afford adding new edge, add it!
                    {
                        nodeAdded = candidatesToPick[i].second;
                        alreadyInPath[nodeAdded] = 1;
                        alreadyChoosed.Add({candidatesToPick[i].first,candidatesToPick[i].second});
                    }
                    break;
                }
            }

            if(pathCost > allMyAnts) break;

            for(int cell = 0 ; cell < numberOfCells; cell++)
            {
                if(cells[cell].resourceType != kEmpty && cells[cell].resourceCount != 0 && alreadyInPath[cell] == 0) candidatesToPick.Add({nodeAdded, cell}); // adding new edges
            }

            int n = candidatesToPick.Size();
            for(int i = n - 1; i >= 0; i--)
            {   
                if(candidatesToPick.Size() == 0) break; // if no edges leave
                if(candidatesToPick[i].second == nodeAdded) // found edge to new created node
                {
                    swap(candidatesToPick[i], candidatesToPick[candidatesToPick.Size() - 1]);
                    candidatesToPick.PopBack();
                }
            }

            placeBeacons();

            int currEval = evalBoard(4);

            if(bestEval == currEval)
            {
                //cerr << bestEval << "\n";
                if(bestEvalPathLength > alreadyChoosed.Size())
                {
                    bestEvalPathLength = alreadyChoosed.Size();
                    bestEval = currEval;
                    bestPath.Reset();
                    //cerr << "NEW BEST but shorter: \n";
                    for(auto edge : alreadyChoosed)
                    {
                        //cerr << edge.first << ' ' << edge.second << "\n";
                        bestPath.Add(edge);
                    }
                }
            }

            if(bestEval < currEval)
            {
                bestEvalPathLength = alreadyChoosed.Size();
                bestEval = currEval;
                bestPath.Reset();
                //cerr << "NEW BEST: \n";
                for(auto edge : alreadyChoosed)
                {
                    //cerr << edge.first << ' ' << edge.second << "\n";
                    bestPath.Add(edge);
                }
                    
            }    
        }
    }

    alreadyChoosed.Reset();
    for(auto edge : bestPath)
    {
        alreadyChoosed.Add(edge);
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

    for(int i = 0 ; i < MAX_EDGE; i++)
    {
        allPairs.Add(test());
        allPairsCopy.Add(test());
        allocations.Add(test());
        moves.Add(test());
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

void upgradePath()
{
    int bestEval = evalBoard(2);
    upgradeSimulations = 0;

    for(int i = 0 ; i < numberOfCells; i++)
        cellsCopy3[i] = cells[i];

    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count() <= timeForUpgrading + timeForChoosing)
    {

        for(int i = 0 ; i < numberOfCells; i++)
            cells[i] = cellsCopy3[i];

        upgradeSimulations++;
        for(int i = 0 ; i < numberOfCells; i++)
        {
            if(cells[i].beacon > 0)
            {
                if(rand() % 100 <= 25)  // 25%
                {
                    int val = (rand() % 7) - 3;
                    cells[i].beacon = max(0, cells[i].beacon + val);
                }
            }
        }
        int currEval = evalBoard(2);
        if(bestEval < currEval)
        {
            bestEval = currEval;
            for(int i = 0 ; i < numberOfCells; i++)
                cellsCopy3[i] = cells[i];
        }   
    }

    for(int i = 0 ; i < numberOfCells; i++)
        cells[i] = cellsCopy3[i];

    cerr << "upgradeSim: " << upgradeSimulations << "\n";
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

        earlyGame = (100.0 * myScore / allCrystal) < 15 && (100.0 * enemyScore / allCrystal) < 15;
        //earlyGame = (100.0 * currentCrystalCount / allCrystal) > 85;

        cerr << "early game? : " << earlyGame << "\n";

        generatePath();

        placeBeacons();

        upgradePath();


        string myMove = "";

        for(int i = 0 ; i < numberOfCells; i++)
        {
            if(cells[i].beacon > 0)
            {
                myMove += "BEACON " + to_string(i) + " " + to_string(cells[i].beacon) + ";";
            }
        }


        cout << myMove << "MESSAGE T: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count() << " S: " << simulations;
        cout << endl;

        timeForChoosing = 65;
        timeForUpgrading = 31;

    }


}