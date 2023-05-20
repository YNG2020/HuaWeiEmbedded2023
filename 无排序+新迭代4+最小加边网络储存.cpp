#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <math.h>
#include <time.h>
const int INF = 2147483647;

using namespace std;

int N, M, T, P, D; // 节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D
const int maxM = 50000; // 边的最大数目
const int maxN = 5000;  // 节点的最大数目
const int maxBus = 10000;   // 业务的最大数目
const int maxP = 80;    // 最大单边通道数量P
int cntEdge = 0;    // 当前边集数组所存储的边的数目
int cntBus = 0;     // 当前业务数组所存储业务的数目

class Node {
public:
    int NodeId;           // 实际上利用数组的下标就可以唯一标识Node，这里先做一个冗余
    int Multiplier[maxP]; // 该节点上存在的放大器，记录的是当前要放大的通道的编号，放大器不存在时值为-1
    vector<int> reachPile;  // 在单次dijkstra搜索中，可抵达该顶点的pile编号
    double passBusCnt;     // 经过这一节点的业务数
}node[maxN];

class Edge {
public:
    int from, to, d, next;    // 起点，终点，边的距离，同起点的下一条边在edge中的编号
    int trueD;    // 边的真正距离
    int Pile[maxP]; // 该边上存在的通道，记录的是当前承载的业务的编号，不承载业务时值为-1
    int usedPileCnt;
    Edge() {
        from = -1;
        to = -1;
        d = 0;
        next = -1;
    }
}edge[maxM];    // 边集数组

class Business {
public:
    int start;  // 业务起点
    int end;    // 业务终点
    int busId;  // 业务Id
    int curA;   // 当前信号强度
    Business() {
        start = -1;
        end = -1;
        pileId = -1;
    }
    int pileId; // 业务所占据的通道Id
    vector<int> pathTmp;   // 存储从起点到其它点的最短路径的末边的编号（考虑通道堵塞的最短）
    vector<int> trueMinPath;   // 存储从起点到其它点的最短路径的末边的编号（不考虑通道堵塞的最短）
    vector<int> path;   // 存储路径所经过的边
    vector<int> mutiplierId;    // 存储所经过的放大器所在节点的编号
    bool isBusWellAllocate; // 标识业余是否被良好分配（业务所经过的边数大于最短所需边数时，标记为false）
}buses[maxBus], copyBuses[maxBus];

int head[maxN]; // head[i]，表示以i为起点的在逻辑上的第一条边在边集数组的位置（编号）
double dis[maxN];  // dis[i]，表示以源点到i到距离
bool vis1[maxN];  // 标识该点有无被访问过
bool vis2[maxN]; // 标识该点有无在添加某业务时，被路径搜索访问过
vector<pair<int, int>> newEdge; // 记录新添加的边的起点和终点
vector<int> newEdgePathId;   // 记录新边在边集中的位置（计数时，双向边视为同一边）
vector<pair<int, int>> copyNewEdge; // 复制最低成本时新添加的边的起点和终点
vector<int> copyNewEdgePathId;   // 复制最低成本时新边在边集中的位置（计数时，双向边视为同一边）
vector<int> remainBus;  // 记录下初次分配时，因路径堵塞而无法分配边的业务的编号
vector<int> totBusIdx;  // 记录所有业务的Id
vector<int> badBusId;   // 记录实际经过的边数大于对应的最短路径的边数的业务
vector<int> goodBusId;  // 记录实际经过的边数等于对应的最短路径的边数的业务
double goodBadGap = 1.5;

struct HashFunc_t {
    size_t operator() (const pair<int, int>& key) const {
        return hash<int>()(key.first) ^ hash<int>()(key.second);;
    }
};

struct Equalfunc_t {
    bool operator() (pair<int, int> const& a, pair<int, int> const& b) const {
        return a.first == b.first && a.second == b.second;
    }
};

unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;   // 记录两个节点间的最短边的距离
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;   // 记录两个节点间的最短路径长度

class Node1 {
public:
    int d, nodeId;

    Node1(int d, int nodeId) {
        this->d = d;
        this->nodeId = nodeId;
    }
    Node1() {}
    bool operator < (const Node1& x) const { // 重载运算符把最小的元素放在堆顶（大根堆）
        if (d > x.d)
            return true;
        else if (d == x.d)
            return node[nodeId].passBusCnt > node[x.nodeId].passBusCnt;
        else
            return false;
        //return d > x.d;
    }
};

class Node2 {
public:
    int nodeId; // 节点编号
    int curLevel;   // 节点到源点的距离
    int usedPileCnt;    // 源点到节点的路径中的边上已经使用了的通道数总和
    Node2(int nodeId, int curLevel, int usedPileCnt) {
        this->nodeId = nodeId;
        this->curLevel = curLevel;
        this->usedPileCnt = usedPileCnt;
    }
    Node2() {}
    bool operator < (const Node2& x) const { // 重载运算符把最大的元素放在堆顶
        return usedPileCnt < x.usedPileCnt;
    }
};

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void dijkstra1(Business& bus);
void dijkstra2(Business& bus);
void dijkstra3(Business& bus);
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath);
bool dijkstra5(Business& bus, int blockEdge);
void dijkstra7(Business& bus);
void BFS1(Business& bus, bool ifLoadNewEdge);
void BFS2(Business& bus);
void BFS3(Business& bus);
bool BFS5(Business& bus, int blockEdge);
bool BFS6(Business& bus, int blockEdge);
void BFS7(Business& bus);
void BFS9(Business& bus);
void loadBus(int busId, bool ifLoadRemain);
void allocateBus();
void reAllocateBus1(int HLim);
void reAllocateBus2(int HLim);
void reAllocateBus3(int HLim);
void reAllocateBus4(int HLim);
void tryDeleteEdge();
void reverseArray(vector<int>& arr);
void outPut();
void outPut2();
bool bfsTestConnection(int start, int end);
void findAddPath(Business& bus, bool* vis2);
void reCoverNetwork(int lastBusID, int lastPileId);
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);
void addMultiplier(Business& bus, int busId);
int testEdgeFull(const vector<int>& path, unordered_set<int>& mark);
int countEdgeNum(const vector<int>& path, unordered_set<int>& mark);
void copyNetwork();

bool ifLast = false;
bool ifTryDeleteEdge = true;

// 主函数
int main() {

    clock_t startTime, curTime, reAllocateTime, tryDeleteTime;
    double reAllocateUnitTime = 0, tryDeleteUnitTime = 0;
    int preNewEdgeNum = INF;
    cin >> N >> M >> T >> P >> D;
    startTime = clock();
    init();
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        cin >> s >> t >> d;
        if (minDist.find(make_pair(s, t)) == minDist.end()) { // 键不存在
            minDist[make_pair(s, t)] = INF;
            minDist[make_pair(t, s)] = INF;
        }
        addEdge(s, t, d);
        addEdge(t, s, d);   // 添加双向边
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        cin >> Sj >> Tj;
        addBus(Sj, Tj); // 添加业务
        BFS3(buses[i]); // 计算最短路径
    }

    allocateBus();
    ifTryDeleteEdge = false;
    curTime = clock();
    int tmpCnt = 0;

    double leftTime = 120 - double(curTime - startTime) / CLOCKS_PER_SEC;

    if (T > 4000 || T <= 3500) {
        while (leftTime > 3 * (tryDeleteUnitTime + reAllocateUnitTime)) {
            ++tmpCnt;
            reAllocateTime = clock();
            reAllocateBus4(0.5*T);
            //reAllocateBus2(0.5*T);
            //reAllocateBus3(0.5 * T);
            curTime = clock();
            reAllocateUnitTime = double(curTime - reAllocateTime) / CLOCKS_PER_SEC;

            tryDeleteTime = clock();
            tryDeleteEdge();

            // 判断成本是否减小
            if (newEdge.size() < preNewEdgeNum) {
                preNewEdgeNum = newEdge.size();
                copyNetwork();
            }

            curTime = clock();
            tryDeleteUnitTime = double(curTime - tryDeleteTime) / CLOCKS_PER_SEC;

            leftTime = 120 - double(curTime - startTime) / CLOCKS_PER_SEC;
            //cout << "reAllocateUnitTime" << reAllocateUnitTime << endl;
            //cout << "tryDeleteUnitTime" << tryDeleteUnitTime << endl;
            //cout << "tmpCnt " << tmpCnt << " cost " << newEdgePathId.size() << endl;
        }
    }
 
    //cout << "leftTime" << leftTime << endl;
    //cout << "tmpCnt" << tmpCnt << endl;
    ifLast = true;
    if (T > 4000 || T <= 3500) {
        tryDeleteEdge();
    }
    else {
        tryDeleteEdge();
        tryDeleteEdge();
    }

    // 判断成本是否减小
    if (newEdge.size() < preNewEdgeNum) {
        preNewEdgeNum = newEdge.size();
        copyNetwork();
    }

    outPut2();

    return 0;
}

// 将所有的业务分配到光网络中
void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i, false);
    }
}

// 试图重新分配业务到光网络中（允许在重分配的过程中加边）
void reAllocateBus1(int HLim) {

    int gap = max(int(0.025 * T), 20);
    if (gap > T)
        return;

    vector<int> busIdx(gap, 0);

    for (int i = 0; i + gap < HLim; i = i + gap) {

        srand(42);  // 设置随机数种子  
        random_shuffle(totBusIdx.begin(), totBusIdx.end());
        for (int i = 0; i < gap; ++i) {
            busIdx[i] = totBusIdx[i];
        }        

        int oriEdgeNum = 0;

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busId; j < i + gap; ++j) {
            busId = busIdx[j - i];
            oriEdgeNum += buses[busId].path.size();
            pathTmp1[j - i] = buses[busId].pathTmp;
            pileTmp1[j - i] = buses[busId].pileId;
            reCoverNetwork(busId, buses[busId].pileId);
        }

        int curEdgeNum = 0;
        bool findPath = false;
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busId; j >= i; --j) {
            busId = busIdx[j - i];
            loadBus(busId, false);
            pileTmp2[j - i] = buses[busId].pileId;
            curEdgeNum += buses[busId].path.size();
        }

        if (1.05 * curEdgeNum < oriEdgeNum) {  // 总体的边数减少，接受迁移
            continue;
        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, busId; j >= i; --j) {   // 把试图寻路时，造成的对网络的影响消除
                busId = busIdx[j - i];
                reCoverNetwork(busId, pileTmp2[j - i]);
            }

            for (int j = i, busId; j < i + gap; ++j) {  // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                busId = busIdx[j - i];
                buses[busId].mutiplierId.swap(nullVector);
                buses[busId].path.swap(nullPath1);
                buses[busId].pathTmp.swap(nullPath2);

                buses[busId].pileId = -1;
                buses[busId].curA = D;
                reloadBus(busId, pileTmp1[j - i], pathTmp1[j - i]);
            }
        }

    }

}

// 试图重新分配业务到光网络中（禁止在重分配的过程中加边）
void reAllocateBus2(int HLim) {

    int gap = max(int(0.05 * T), 20);
    if (gap > T)
        return;
    vector<int> totBusIdx(T, 0);
    vector<int> busIdx(gap, 0);
    for (int i = 0; i < T; ++i)
        totBusIdx[i] = i;

    for (int i = 0; i + gap < HLim; i = i + gap) {

        srand(time(NULL));  // 设置随机数种子  
        random_shuffle(totBusIdx.begin(), totBusIdx.end());
        for (int k = 0; k < gap; ++k) {
            busIdx[k] = totBusIdx[k];
        }

        int oriEdgeNum = 0;

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busId; j < i + gap; ++j) {
            busId = busIdx[j - i];
            oriEdgeNum += buses[busId].path.size();
            pathTmp1[j - i] = buses[busId].pathTmp;
            pileTmp1[j - i] = buses[busId].pileId;
            reCoverNetwork(busId, buses[busId].pileId);
        }

        int curEdgeNum = 0;
        bool findPath = false;
        vector<vector<int>> pathTmp2(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp2(gap, -1);

        for (int j = i + gap - 1, busId; j >= i; --j) {

            busId = busIdx[j - i];
            findPath = BFS5(buses[busId], -1);
            if (findPath) {
                pathTmp2[j - i] = buses[busId].pathTmp;
                pileTmp2[j - i] = buses[busId].pileId;
                curEdgeNum += buses[busId].path.size();

                int curNode = buses[busId].start, trueNextEdgeId;
                for (int i = 0; i < buses[busId].path.size(); ++i) {

                    if (edge[buses[busId].path[i] * 2].from == curNode)
                        trueNextEdgeId = buses[busId].path[i] * 2;
                    else
                        trueNextEdgeId = buses[busId].path[i] * 2 + 1;
                    curNode = edge[trueNextEdgeId].to;

                    if (buses[busId].curA >= edge[trueNextEdgeId].trueD) {
                        buses[busId].curA -= edge[trueNextEdgeId].trueD;
                    }
                    else {
                        node[edge[trueNextEdgeId].from].Multiplier[buses[busId].pileId] = buses[busId].pileId;
                        buses[busId].curA = D;
                        buses[busId].curA -= edge[trueNextEdgeId].trueD;
                        buses[busId].mutiplierId.push_back(edge[trueNextEdgeId].from);
                    }
                }

            }
            else {  // BFS5找不到路时，不会对网络和业务产生任何影响
                findPath = false;
                break;
            }

        }

        if (findPath && 1.025 * curEdgeNum < oriEdgeNum) {  // 总体的边数减少，接受迁移
            continue;
        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, busId; j >= i; --j) {   // 把试图寻路时，造成的对网络的影响消除     
                if (pathTmp2[j - i].size() == 0)        // 该业务寻路失败，对网络不造成影响
                    continue;
                busId = busIdx[j - i];
                reCoverNetwork(busId, pileTmp2[j - i]);
            }

            for (int j = i, busId; j < i + gap; ++j) {  // 重新加载所有的边

                vector<int> nullVector, nullPath1, nullPath2;
                busId = busIdx[j - i];
                buses[busId].mutiplierId.swap(nullVector);
                buses[busId].path.swap(nullPath1);
                buses[busId].pathTmp.swap(nullPath2);

                buses[busId].pileId = -1;
                buses[busId].curA = D;
                reloadBus(busId, pileTmp1[j - i], pathTmp1[j - i]);
            }
        }

    }


}

// 试图重新分配业务到光网络中（允许在重分配的过程中加边，挑选出的要重分配的业务尽可能是“不良好”分配的）
void reAllocateBus3(int HLim) {

    int gap = max(int(0.025 * T), 20);
    if (gap > T)
        return;

    double ratio = 0.5;     // 以ratio的概率，选择不良好的业务进行分配
    vector<int> goodBusId;  // 良好分配的业务，“良好”的定义是该业务所经过的路径长度小于最短路径长度*goofBadGap
    vector<int> badBusId;   // 不良好分配的业务
    for (int i = 0; i < T; ++i)
        if (buses[i].isBusWellAllocate)
            goodBusId.push_back(i);
        else
            badBusId.push_back(i);

    vector<int> busIdx(gap, 0);

    for (int i = 0; i + gap < HLim; i = i + gap) {

        srand(time(NULL) + rand() % 1000);  // 设置随机数种子  
        random_shuffle(goodBusId.begin(), goodBusId.end());
        random_shuffle(badBusId.begin(), badBusId.end());
        int goodBusSize = goodBusId.size(), badBusSize = badBusId.size();
        
        for (int j = 0, m = 0, n = 0; j < gap; ++j) {
            
            if (n >= goodBusSize) {
                busIdx[j] = badBusId[m++];
                continue;
            }
            if (m >= badBusSize) {
                busIdx[j] = goodBusId[n++];
                continue;
            }
            double curRatio = (double)(rand()) / RAND_MAX;
            if (curRatio < ratio)   // 以ratio的概率，选择不良好的业务进行分配
                busIdx[j] = badBusId[m++];
            else
                busIdx[j] = goodBusId[n++];
        }

        int oriEdgeNum = 0;
        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busId; j < i + gap; ++j) {
            busId = busIdx[j - i];
            oriEdgeNum += buses[busId].path.size();
            pathTmp1[j - i] = buses[busId].pathTmp;
            pileTmp1[j - i] = buses[busId].pileId;
            reCoverNetwork(busId, buses[busId].pileId);
        }

        int oriCntEdge = cntEdge;
        int curEdgeNum = 0;
        bool findPath = false;
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busId; j >= i; --j) {
            busId = busIdx[j - i];
            loadBus(busId, false);
            pileTmp2[j - i] = buses[busId].pileId;
            curEdgeNum += buses[busId].path.size();
        }

        if (1.05 * curEdgeNum < oriEdgeNum) {  // 总体的边数减少，接受迁移
            continue;
        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, busId; j >= i; --j) {   // 把试图寻路时，造成的对网络的影响消除
                busId = busIdx[j - i];
                reCoverNetwork(busId, pileTmp2[j - i]);
            }

            for (int j = i, busId; j < i + gap; ++j) {  // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                busId = busIdx[j - i];
                buses[busId].mutiplierId.swap(nullVector);
                buses[busId].path.swap(nullPath1);
                buses[busId].pathTmp.swap(nullPath2);

                buses[busId].pileId = -1;
                buses[busId].curA = D;
                reloadBus(busId, pileTmp1[j - i], pathTmp1[j - i]);
            }

            // 在这里利用cntEdge的变化来及时删除无用新边
            //for (int j = oriCntEdge; j < cntEdge; j = j + 2) {

            //    int iter = find(newEdgePathId.begin(), newEdgePathId.end(), j / 2) - newEdgePathId.begin();
            //    newEdge.erase(newEdge.begin() + iter);
            //    newEdgePathId.erase(newEdgePathId.begin() + iter);

            //    for (int k = 0; k < P; ++k) {   // 该边已删除，就应对其进行封锁
            //        edge[j].Pile[k] = T;
            //        edge[j + 1].Pile[k] = T;   // 偶数+1
            //    }
            //}
        }

    }

}

// 试图重新分配业务到光网络中（允许在重分配的过程中加边）
void reAllocateBus4(int HLim) {

    int gap = max(int(0.025 * T), 20);
    if (gap > T)
        return;

    vector<int> busIdx(gap, 0);

    for (int i = 0; i + gap < HLim; i = i + gap) {

        srand(42);  // 设置随机数种子  
        random_shuffle(totBusIdx.begin(), totBusIdx.end());
        for (int i = 0; i < gap; ++i) {
            busIdx[i] = totBusIdx[i];
        }

        int oriEdgeNum = 0, oriFullEdge = 0;
        unordered_set<int> mark1;
        unordered_set<int> mark2;
        for (int j = i; j < i + gap; ++j) {
            oriEdgeNum += countEdgeNum(buses[busIdx[j - i]].path, mark1);
            oriFullEdge += testEdgeFull(buses[busIdx[j - i]].path, mark2);
        }

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busId; j < i + gap; ++j) {
            busId = busIdx[j - i];
            pathTmp1[j - i] = buses[busId].pathTmp;
            pileTmp1[j - i] = buses[busId].pileId;
            reCoverNetwork(busId, buses[busId].pileId);
        }

        int curEdgeNum = 0, curFullEdge = 0;
        bool findPath = false;
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busId; j >= i; --j) {
            busId = busIdx[j - i];
            loadBus(busId, false);
            pileTmp2[j - i] = buses[busId].pileId;
            curEdgeNum += buses[busId].path.size();
        }
        unordered_set<int> mark3;
        unordered_set<int> mark4;
        for (int j = i; j < i + gap; ++j) {
            curEdgeNum += countEdgeNum(buses[busIdx[j - i]].path, mark3);
            curFullEdge += testEdgeFull(buses[busIdx[j - i]].path, mark4);
        }

        if (curFullEdge * oriEdgeNum < oriFullEdge * curEdgeNum) {  // 边的利用效率下降，接受迁移
            continue;

        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, busId; j >= i; --j) {   // 把试图寻路时，造成的对网络的影响消除
                busId = busIdx[j - i];
                reCoverNetwork(busId, pileTmp2[j - i]);
            }

            for (int j = i, busId; j < i + gap; ++j) {  // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                busId = busIdx[j - i];
                buses[busId].mutiplierId.swap(nullVector);
                buses[busId].path.swap(nullPath1);
                buses[busId].pathTmp.swap(nullPath2);

                buses[busId].pileId = -1;
                buses[busId].curA = D;
                reloadBus(busId, pileTmp1[j - i], pathTmp1[j - i]);
            }
        }

    }

}

// 试图删除新边
void tryDeleteEdge() {

    int n = newEdge.size(), trueEdgeId;
    for (int idx = 0; idx < n; ++idx) {
        int idxEdge = newEdgePathId[idx]; // idxEdge为边在边集数组的编号（计数时，双向边视作同一边）  

        trueEdgeId = idxEdge * 2;
        int busCnt = 0;     // 记录边上承载的业务数
        vector<int> lastBusIds, lastPileIds;

        for (int j = 0; j < P; ++j)
            if (edge[trueEdgeId].Pile[j] != -1 && edge[trueEdgeId].Pile[j] != T) {   // 说明在通道j上承载了该业务
                ++busCnt;
                lastBusIds.push_back(edge[trueEdgeId].Pile[j]);
                lastPileIds.push_back(j);
            }

        if (busCnt == 0) {      // 如果该新边上，一条业务都没有承载，直接删边

            int iter = find(newEdgePathId.begin(), newEdgePathId.end(), idxEdge) - newEdgePathId.begin();
            newEdge.erase(newEdge.begin() + iter);
            newEdgePathId.erase(newEdgePathId.begin() + iter);
            --idx;
            --n;

            for (int k = 0; k < P; ++k) {   // 该边已删除，就应对其进行封锁
                edge[trueEdgeId].Pile[k] = T;
                edge[trueEdgeId + 1].Pile[k] = T;   // 偶数+1
            }
        }
        else {  // 如果该新边上，承载了多条业务，则对该边上的所有业务重新分配，看能否不依赖该新边

            vector<vector<int>> pathTmp(busCnt, vector<int>());     // 用于此后重新加载边
            for (int k = 0; k < busCnt; ++k) {
                pathTmp[k] = buses[lastBusIds[k]].pathTmp;
                reCoverNetwork(lastBusIds[k], lastPileIds[k]);
            }

            bool findPath = false;
            int stopK = -1;
            vector<int> tmpLastPileIds;
            for (int k = 0; k < busCnt; ++k) {

                findPath = BFS5(buses[lastBusIds[k]], idxEdge);
                //findPath = dijkstra5(buses[lastBusIds[k]], idxEdge);

                if (findPath == false) {
                    stopK = k;
                    break;
                }
                tmpLastPileIds.push_back(buses[lastBusIds[k]].pileId);   // 原本的pileId已改变，此处进行更新，以防止reCoverNetwork时出bug
            }

            if (findPath) {

                int iter = find(newEdgePathId.begin(), newEdgePathId.end(), idxEdge) - newEdgePathId.begin();
                newEdge.erase(newEdge.begin() + iter);
                newEdgePathId.erase(newEdgePathId.begin() + iter);
                --idx;
                --n;

                for (int k = 0; k < P; ++k) {   // 该边已删除，就应对其进行封锁
                    edge[trueEdgeId].Pile[k] = T;
                    edge[trueEdgeId + 1].Pile[k] = T;   // 偶数+1
                }

                for (int k = 0; k < busCnt; ++k) {

                    vector<int> nullVector;
                    buses[lastBusIds[k]].mutiplierId.swap(nullVector);
                    buses[lastBusIds[k]].curA = D;

                    addMultiplier(buses[lastBusIds[k]], lastBusIds[k]);

                }

            }
            else {

                for (int k = 0; k < stopK; ++k) {   // 把试图寻路时，造成的对网络的影响消除
                    reCoverNetwork(lastBusIds[k], tmpLastPileIds[k]);
                }

                for (int k = 0; k < busCnt; ++k) {  // 重新加载所有的边
                    vector<int> nullVector, nullPath1, nullPath2;
                    buses[lastBusIds[k]].mutiplierId.swap(nullVector);
                    buses[lastBusIds[k]].path.swap(nullPath1);
                    buses[lastBusIds[k]].pathTmp.swap(nullPath2);

                    buses[lastBusIds[k]].pileId = -1;
                    buses[lastBusIds[k]].curA = D;
                    reloadBus(lastBusIds[k], lastPileIds[k], pathTmp[k]);
                }
            }

        }

    }

}

// 把业务busId加载到光网络中
void loadBus(int busId, bool ifLoadRemain) {

    //BFS1(buses[busId], false);
    dijkstra1(buses[busId]);
    addMultiplier(buses[busId], busId);
}

// 初始化
void init() {

    for (int i = 0; i < N; ++i) {

        head[i] = -1;
        node[i].NodeId = i;
        node[i].passBusCnt = 0;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }

    for (int i = 0; i < T; ++i)
        buses[i].curA = D;

    totBusIdx.resize(T, 0);
    for (int i = 0; i < T; ++i)
        totBusIdx[i] = i;

}

// 加边函数，s起点，t终点，d距离
void addEdge(int s, int t, int d) {
    edge[cntEdge].from = s; // 起点
    edge[cntEdge].to = t;   // 终点
    edge[cntEdge].d = 1;    // 距离
    //edge[cntEdge].d = d;    // 距离
    edge[cntEdge].trueD = d;    // 距离
    edge[cntEdge].next = head[s];   // 链式前向。以s为起点下一条边的编号，head[s]代表的是当前以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    for (int i = 0; i < P; ++i)
        edge[cntEdge].Pile[i] = -1;
    edge[cntEdge].usedPileCnt = 0;
    head[s] = cntEdge++;    // 更新以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
}

// 加业务函数
void addBus(int start, int end) {
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    vector<int>().swap(buses[cntBus].path);
    vector<int>().swap(buses[cntBus].pathTmp);
    vector<int>().swap(buses[cntBus].mutiplierId);
    ++cntBus;
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最短路径，因为有可能边的通道被完全占用）
void dijkstra1(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    static int addNewBus = 0;   // 加业务次数
    ++addNewBus;
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    double maxValue = -1;

    if (addNewBus % 2)
        p = 0;
    else
        p = P - 1;

    while (true) {
        if (addNewBus % 2) {
            ++p;
            if (p >= P)
                break;
        }
        else {
            --p;
            if (p < 0)
                break;
        }

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            dis[i] = INF;
            vis1[i] = false;
        }
        dis[start] = 0;  // 源点到源点的距离为0
        priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身
        q.push(Node1(0, start));
        int s = -1;
        while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了

            Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
            q.pop();
            s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

            if (s == end) // 当end已经加入到了生成树，则结束搜索
                break;

            // 没有遍历过才需要遍历
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;

                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                        dis[t] = dis[s] + edge[i].d;   // 松弛操作
                        q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中
                    }
                }

            }
        }
        if (s == end) { // 当end已经加入到了生成树，则结束搜索

            //if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // 键不存在
            //    minPathSize[make_pair(start, end)] = dis[s];
            //else if (minPathSize[make_pair(start, end)] > dis[s])
            //    minPathSize[make_pair(start, end)] = dis[s];

            if (minPathSize[make_pair(start, end)] * goodBadGap > dis[s])
                bus.isBusWellAllocate = true;
            else
                bus.isBusWellAllocate = false;

            if (dis[s] > 3 * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            int curNode = end, tmpDist = dis[s];
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // 找不到路，需要构造新边

        if (ifTryDeleteEdge) {
            if (T > 3500 && T <= 4000) {
                if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                    tryDeleteEdge();
            }
            else
                if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                    tryDeleteEdge();
        }

        dijkstra7(bus);
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;
        
        if (edgeId % 2) {   // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
}

// 寻找业务bus的起点到终点的路径，并对路径上的每一条边都执行加边操作，然后交给dijkstra1操作
void dijkstra2(Business& bus) {

    int start = bus.start, end = bus.end;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // 源点到源点的距离为0

    priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
        Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
        q.pop();
        s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

        if (s == end)   // 当end已经加入到了生成树，则结束搜索
            break;

        // 没有遍历过才需要遍历
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                bus.trueMinPath[t] = i;    // 记录下抵达路径点t的边的编号i
                dis[t] = dis[s] + edge[i].d;   // 松弛操作
                q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中  
            }
        }
    }

    int curNode = end;
    while (bus.trueMinPath[curNode] != -1) {
        int edgeId = bus.trueMinPath[curNode];  // 存储于edge数组中真正的边的Id

        addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
        addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

        if (edge[edgeId].from < edge[edgeId].to)
            newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
        else
            newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
        newEdgePathId.emplace_back(cntEdge / 2 - 1);

        curNode = edge[bus.trueMinPath[curNode]].from;
    }
    dijkstra1(bus);
    return;
}

// 寻找业务bus的起点到终点，在不考虑通道堵塞下的最短路径
void dijkstra3(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    if (minPathSize.find(make_pair(start, end)) != minPathSize.end())   // 键存在，无需再次查找
        return;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // 源点到源点的距离为0

    priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
        Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
        q.pop();
        s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

        if (s == end) { // 当end已经加入到了生成树，则结束搜索
            bus.pileId = p;
            break;
        }

        // 没有遍历过才需要遍历
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                bus.trueMinPath[t] = i;    // 记录下抵达路径点t的边的编号i
                dis[t] = dis[s] + edge[i].d;   // 松弛操作
                q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中  
            }
        }
    }
    minPathSize[make_pair(start, end)] = dis[end];
}

// 寻找从start到end，在特定通道下的最短路径
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath) {

    if (start == end)
        return;
    for (int i = 0; i < N; ++i) { // 赋初值
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // 源点到源点的距离为0

    priority_queue<Node1> null_queue; // 定义一个空的priority_queue对象
    priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身
    q.swap(null_queue);// 交换队列容器中的内容
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
        Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
        q.pop();
        s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

        if (s == end) { // 当end已经加入到了生成树，则结束搜索
            break;
        }

        // 没有遍历过才需要遍历
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边

            if (edge[i].Pile[pileId] == -1) {
                int t = edge[i].to;
                if (dis[t] > dis[s] + edge[i].d) {
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                    dis[t] = dis[s] + edge[i].d;   // 松弛操作
                    q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中  
                }
            }

        }
    }

}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回
bool dijkstra5(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            dis[i] = INF;
            vis1[i] = false;
        }
        dis[start] = 0;  // 源点到源点的距离为0
        priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身
        q.push(Node1(0, start));
        int s = -1;
        while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
            Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
            q.pop();
            s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

            if (s == end)   // 当end已经加入到了生成树，则结束搜索
                break;

            // 没有遍历过才需要遍历
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边

                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                        dis[t] = dis[s] + edge[i].d;   // 松弛操作
                        q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中
                    }
                }

            }
        }
        if (s == end) { // 当end已经加入到了生成树，则结束搜索

            if (!ifLast && dis[s] > 3 * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            if (minPathSize[make_pair(start, end)] == dis[s])
                bus.isBusWellAllocate = true;
            else
                bus.isBusWellAllocate = false;

            int curNode = end, tmpDist = dis[s];
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // 找不到路，直接返回
        return false;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
    return true;

}

// 不能加载业务时，寻找业务bus的起点到终点的路径（不考虑通道堵塞，全通道搜索），并对路径上的发生堵塞的边进行加边处理
void dijkstra7(Business& bus) {

    int start = bus.start, end = bus.end;
    int minBlockEdge = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    tmpOKPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // 源点到源点的距离为0

    priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
        Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
        q.pop();
        s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

        if (s == end)   // 当end已经加入到了生成树，则结束搜索
            break;

        // 没有遍历过才需要遍历
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                dis[t] = dis[s] + edge[i].d;   // 松弛操作
                q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中  
            }
        }
    }

    for (int p = 0; p < P; ++p) {

        int curNode = end, tmpBlockEdge = 0;
        while (tmpOKPath[curNode] != -1) {
            int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
            if (edge[edgeId].Pile[p] != -1)
                ++tmpBlockEdge;
            curNode = edge[edgeId].from;
        }

        if (tmpBlockEdge < minBlockEdge) {   // 选需要加边数最少的通道
            minBlockEdge = tmpBlockEdge;
            bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            choosenP = p;
        }

    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        int lastNode = curNode;
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1
        curNode = edge[bus.pathTmp[curNode]].from;

        if (edge[edgeId].Pile[choosenP] == -1) {    // 无需加边
            bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
            edge[edgeId].Pile[choosenP] = bus.busId;
            ++edge[edgeId].usedPileCnt;

            if (edgeId % 2) {   // 奇数-1
                edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
                ++edge[edgeId - 1].usedPileCnt;
            }
            else {  // 偶数+1
                edge[edgeId + 1].Pile[choosenP] = bus.busId;
                ++edge[edgeId + 1].usedPileCnt;
            }
        }
        else {      // 需要加边
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
            newEdgePathId.emplace_back(cntEdge / 2 - 1);

            bus.path.push_back(cntEdge / 2 - 1); // edgeId / 2是为了适应题目要求
            edge[cntEdge - 2].Pile[choosenP] = bus.busId;
            ++edge[cntEdge - 2].usedPileCnt;
            edge[cntEdge - 1].Pile[choosenP] = bus.busId;   // 偶数+1
            ++edge[cntEdge - 1].usedPileCnt;
            bus.pathTmp[lastNode] = cntEdge - 2;
        }

    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
    bus.isBusWellAllocate = true;

}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void BFS1(Business& bus, bool ifLoadNewEdge) {

    int start = bus.start, end = bus.end, p = 0;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    static int addNewBus = 0;   // 加业务次数
    ++addNewBus;
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    double maxValue = -1;

    if (addNewBus % 2)
        p = 0;
    else
        p = P - 1;

    while (true) {
        if (addNewBus % 2) {
            ++p;
            if (p >= P)
                break;
        }
        else {
            --p;
            if (p < 0)
                break;
        }

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        //queue<Node2> bfsq;
        bfsQ.push(make_pair(start, 0));
        //bfsq.push(Node2(start, 0, 0));
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        //int totUsedPileCnt = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) {

            //Node2 tmpNode = bfsq.front();
            s = bfsQ.front().first;
            //s = tmpNode.nodeId;
            curLevel = bfsQ.front().second;
            //curLevel = tmpNode.curLevel;
            //totUsedPileCnt = tmpNode.usedPileCnt;
            bfsQ.pop();
            //bfsq.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        ++curLevel;
                        //totUsedPileCnt += edge[i].usedPileCnt;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                        //bfsq.push(Node2(t, curLevel + 1, totUsedPileCnt + edge[i].usedPileCnt));
                    }

                }
            }

        }
        if (s == end) {

            //if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // 键不存在
            //    minPathSize[make_pair(start, end)] = curLevel;
            //else if (minPathSize[make_pair(start, end)] > curLevel) {
            //    minPathSize[make_pair(start, end)] = curLevel;
            //}

            if (minPathSize[make_pair(start, end)] * goodBadGap > curLevel)
                bus.isBusWellAllocate = true;
            else
                bus.isBusWellAllocate = false;

            if (ifLoadNewEdge) {    // 如果BFS1在调用前已经添加了新边，则可以一遍过
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
                findPath = true;
                break;
            }

            if (curLevel > 3 * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            int curNode = end, tmpDist = curLevel;
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {

        if (ifTryDeleteEdge) {
            if (T > 3500 && T <= 4000) {
                if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                    tryDeleteEdge();
            }
            else
                if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                    tryDeleteEdge();
        }

        //BFS2(bus);       // 旧的加边策略，一但加边，整个路径都会加，但全局性能是当前最好的
        BFS7(bus);       // 新加边策略，只加最短路径上需要进行加边的边
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
}

// 寻找业务bus的起点到终点的路径（不考虑通道堵塞），并对路径上的每一条边都执行加边操作，然后交给BFS1操作
void BFS2(Business& bus) {

    int start = bus.start, end = bus.end;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }

    queue<pair<int, int>> bfsQ;
    bfsQ.push(make_pair(start, 0));
    vis1[start] = true;
    int s = start;
    int curLevel = 0;
    bool getOutFlag = false;

    while (!bfsQ.empty() && !getOutFlag) {

        s = bfsQ.front().first;
        curLevel = bfsQ.front().second;
        bfsQ.pop();

        for (int i = head[s]; i != -1; i = edge[i].next) {
            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            bus.trueMinPath[t] = i;    // 记录下抵达路径点t的边的编号i
            if (t == end) {
                getOutFlag = true;
                s = t;
                break;
            }
            else {
                bfsQ.push(make_pair(t, curLevel + 1));
            }
        }

    }

    int curNode = end;
    while (bus.trueMinPath[curNode] != -1) {
        int edgeId = bus.trueMinPath[curNode];  // 存储于edge数组中真正的边的Id

        addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
        addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

        if (edge[edgeId].from < edge[edgeId].to)
            newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
        else
            newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
        newEdgePathId.emplace_back(cntEdge / 2 - 1);

        curNode = edge[bus.trueMinPath[curNode]].from;
    }
    BFS1(bus, true);

}

// 寻找业务bus的起点到终点，在不考虑通道堵塞下的最短路径
void BFS3(Business& bus) {

    int start = bus.start, end = bus.end;

    if (minPathSize.find(make_pair(start, end)) != minPathSize.end())   // 键存在，无需再次查找
        return;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }

    queue<pair<int, int>> bfsQ;
    bfsQ.push(make_pair(start, 0));
    vis1[start] = true;
    int s = start;
    int curLevel = 0;
    bool getOutFlag = false;

    while (!bfsQ.empty() && !getOutFlag) {

        s = bfsQ.front().first;
        curLevel = bfsQ.front().second;
        bfsQ.pop();

        for (int i = head[s]; i != -1; i = edge[i].next) {
            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            bus.trueMinPath[t] = i;    // 记录下抵达路径点t的边的编号i
            if (t == end) {
                getOutFlag = true;
                s = t;
                ++curLevel;
                break;
            }
            else {
                bfsQ.push(make_pair(t, curLevel + 1));
            }
        }

    }
    minPathSize[make_pair(start, end)] = curLevel;

}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回
bool BFS5(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) {

            s = bfsQ.front().first;
            curLevel = bfsQ.front().second;
            bfsQ.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        ++curLevel;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (s == end) {

            // 找到的路径长度太长，宁愿不要
            if (!ifLast && (curLevel > 3 * minPathSize[make_pair(start, end)]))
                continue;

            if (minPathSize[make_pair(start, end)] == curLevel)
                bus.isBusWellAllocate = true;
            else
                bus.isBusWellAllocate = false;

            int curNode = end, tmpDist = curLevel;
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // 找不到路，需要构造新边
        return false;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
    return true;
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回，找到路径后，也直接返回，不作其它处理
bool BFS6(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) {

            s = bfsQ.front().first;
            curLevel = bfsQ.front().second;
            bfsQ.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        return true;
                        ++curLevel;
                        break;
                    }
                    else
                        bfsQ.push(make_pair(t, curLevel + 1));

                }
            }

        }

    }

    return false;

}

// 不能加载业务时，寻找业务bus的起点到终点的路径（不考虑通道堵塞，全通道搜索），并对路径上的发生堵塞的边进行加边处理
void BFS7(Business& bus) {

    int start = bus.start, end = bus.end;
    int minBlockEdge = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    tmpOKPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }
    queue<pair<int, int>> bfsQ;
    bfsQ.push(make_pair(start, 0));
    vis1[start] = true;
    int s = start;
    int curLevel = 0;
    bool getOutFlag = false;

    while (!bfsQ.empty() && !getOutFlag) {

        s = bfsQ.front().first;
        curLevel = bfsQ.front().second;
        bfsQ.pop();

        for (int i = head[s]; i != -1; i = edge[i].next) {

            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

            if (t == end) {
                getOutFlag = true;
                s = t;
                ++curLevel;
                break;
            }
            else
                bfsQ.push(make_pair(t, curLevel + 1));
        }

    }
    for (int p = 0; p < P; ++p) {

        int curNode = end, tmpBlockEdge = 0;
        while (tmpOKPath[curNode] != -1) {
            int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
            if (edge[edgeId].Pile[p] != -1)
                ++tmpBlockEdge;
            curNode = edge[edgeId].from;
        }

        if (tmpBlockEdge < minBlockEdge) {   // 选需要加边数最少的通道
            minBlockEdge = tmpBlockEdge;
            bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            choosenP = p;
        }

    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        int lastNode = curNode;
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1
        curNode = edge[bus.pathTmp[curNode]].from;

        if (edge[edgeId].Pile[choosenP] == -1) {    // 无需加边
            bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
            edge[edgeId].Pile[choosenP] = bus.busId;
            ++edge[edgeId].usedPileCnt;

            if (edgeId % 2) {   // 奇数-1
                edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
                ++edge[edgeId - 1].usedPileCnt;
            }
            else {  // 偶数+1
                edge[edgeId + 1].Pile[choosenP] = bus.busId;
                ++edge[edgeId + 1].usedPileCnt;
            }
        }
        else {      // 需要加边
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
            newEdgePathId.emplace_back(cntEdge / 2 - 1);

            bus.path.push_back(cntEdge / 2 - 1); // edgeId / 2是为了适应题目要求
            edge[cntEdge - 2].Pile[choosenP] = bus.busId;
            ++edge[cntEdge - 2].usedPileCnt;
            edge[cntEdge - 1].Pile[choosenP] = bus.busId;   // 偶数+1
            ++edge[cntEdge - 1].usedPileCnt;
            bus.pathTmp[lastNode] = cntEdge - 2;
        }

    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
    bus.isBusWellAllocate = true;
}

// 统计了路径上的边所使用的总通道数。但这个统计量没有使用到。考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void BFS9(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            vis1[i] = false;
        }
        priority_queue<Node2>bfsq;
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        bfsq.emplace(start, 0, 0);
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        int totUsedPileCnt = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) {

            Node2 tmpNode = bfsq.top();
            s = bfsQ.front().first;
            s = tmpNode.nodeId;
            curLevel = bfsQ.front().second;
            curLevel = tmpNode.curLevel;
            totUsedPileCnt = tmpNode.usedPileCnt;
            bfsQ.pop();
            bfsq.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        ++curLevel;
                        totUsedPileCnt += edge[i].usedPileCnt;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                        bfsq.emplace(t, curLevel + 1, totUsedPileCnt + edge[i].usedPileCnt);
                    }

                }
            }

        }
        if (s == end) {

            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // 键不存在
                minPathSize[make_pair(start, end)] = curLevel;
            else if (minPathSize[make_pair(start, end)] > curLevel) {
                minPathSize[make_pair(start, end)] = curLevel;
            }

            if (curLevel > 3 * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            int curNode = end, tmpDist = curLevel;
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // 找不到路，需要构造新边

        if (T > 3500 && T <= 4000) {
            if (++addNewEdgeCnt % 2 == 0 && (bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                tryDeleteEdge();
        }
        else
            if ((bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                tryDeleteEdge();

        BFS7(bus);       // 新加边策略，只加最短路径上需要进行加边的边
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(bus.path);
}

// 反转数组
void reverseArray(vector<int>& arr) {

    int tmp, n = arr.size();
    for (int i = 0; i < n / 2; ++i) {
        tmp = arr[n - i - 1];
        arr[n - i - 1] = arr[i];
        arr[i] = tmp;
    }

}

// 将结果输出
void outPut() {

    unordered_map<int, int> newEdgeMap;
    int n = newEdge.size();
    if (n > 0) {
        newEdgeMap[newEdgePathId[0]] = M;
        for (int i = 1; i < n; ++i) {
            newEdgeMap[newEdgePathId[i]] = newEdgeMap[newEdgePathId[i - 1]] + 1;
        }
    }

    cout << newEdge.size() << endl;
    for (int i = 0; i < newEdge.size(); ++i) {
        cout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();

        cout << buses[i].pileId << " " << pSize << " "
            << mSize << " ";
        for (int j = 0; j < pSize; ++j) {

            int pathID = buses[i].path[j];
            if (pathID >= M)
                pathID = newEdgeMap[pathID];

            cout << pathID;
            if (mSize == 0 && j == pSize - 1 && i != T - 1) {
                cout << endl;
            }
            else if (mSize != 0 || j != pSize - 1) {
                cout << " ";
            }
        }
        for (int j = 0; j < mSize; ++j) {
            cout << buses[i].mutiplierId[j];
            if (j < buses[i].mutiplierId.size() - 1)
                cout << " ";
            else if (j == mSize - 1 && i != T - 1)
                cout << endl;
        }
    }
}

// 输出复制网络
void outPut2() {

    unordered_map<int, int> newEdgeMap;
    int n = copyNewEdge.size();
    if (n > 0) {
        newEdgeMap[copyNewEdgePathId[0]] = M;
        for (int i = 1; i < n; ++i) {
            newEdgeMap[copyNewEdgePathId[i]] = newEdgeMap[copyNewEdgePathId[i - 1]] + 1;
        }
    }

    cout << copyNewEdge.size() << endl;
    for (int i = 0; i < copyNewEdge.size(); ++i) {
        cout << copyNewEdge[i].first << " " << copyNewEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = copyBuses[i].path.size();
        int mSize = copyBuses[i].mutiplierId.size();

        cout << copyBuses[i].pileId << " " << pSize << " "
            << mSize << " ";
        for (int j = 0; j < pSize; ++j) {

            int pathID = copyBuses[i].path[j];
            if (pathID >= M)
                pathID = newEdgeMap[pathID];

            cout << pathID;
            if (mSize == 0 && j == pSize - 1 && i != T - 1) {
                cout << endl;
            }
            else if (mSize != 0 || j != pSize - 1) {
                cout << " ";
            }
        }
        for (int j = 0; j < mSize; ++j) {
            cout << copyBuses[i].mutiplierId[j];
            if (j < copyBuses[i].mutiplierId.size() - 1)
                cout << " ";
            else if (j == mSize - 1 && i != T - 1)
                cout << endl;
        }
    }
}

// 测试start与end两点之间的连通性，连通则返回true
bool bfsTestConnection(int start, int end) {

    if (start == end)
        return true;
    vector<bool> vis3(N, false);
    vis3[start] = true;
    queue<int> q;
    q.push(start);

    while (!q.empty()) {

        int curNode = q.front();
        q.pop();
        for (int i = head[curNode]; i != -1; i = edge[i].next) {
            int t = edge[i].to;
            if (vis3[t])
                continue;
            vis3[t] = true;
            if (t == end)
                return true;
            else {
                q.push(t);
            }
        }

    }
    return false;

}

// 在因通道堵塞而添加不了业务时，寻找合适的位置进行加边操作，并再次寻路
void findAddPath(Business& bus, bool* vis2) {

    int end = bus.end;  // end点在此之前保证不可达
    vector<bool> vis3(N, false);
    vis3[end] = true;
    queue<int> q;
    q.push(end);
    int firstOKPoint = bus.start;
    int OKPile = -1;
    vector<int> tmpOKPath(N, -1);   // 存储路径，内容是到每一个点的前一条边的编号

    bool getOutFlag = false;
    while (!q.empty() && !getOutFlag) {    // 寻找end到第一个可达点的路径

        int curNode = q.front();
        q.pop();
        for (int i = head[curNode]; i != -1; i = edge[i].next) {

            int t = edge[i].to;
            if (vis3[t])
                continue;
            vis3[t] = true;
            tmpOKPath[t] = i;

            if (vis2[t]) {
                firstOKPoint = t;
                if (t == bus.start)
                    OKPile = 0;
                else
                    OKPile = node[t].reachPile[0];
                getOutFlag = true;
                break;
            }
            else {  // 说明t点也是不可达点
                q.push(t);
            }

        }
    }

    dijkstra4(firstOKPoint, bus.start, OKPile, tmpOKPath);  // tmpOKPath存储的路径是从终点到起点的路径，使用时要注意

    int curNode = bus.start;
    bus.pileId = OKPile;
    bool addEdgeFlag = false;
    while (tmpOKPath[curNode] != -1) {
        int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id

        if (curNode == firstOKPoint) {
            addEdgeFlag = true;
        }

        if (addEdgeFlag) {
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);

            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
            newEdgePathId.emplace_back(cntEdge / 2 - 1);

            bus.path.push_back((cntEdge - 1) / 2); // edgeId / 2是为了适应题目要求
            edge[(cntEdge - 1)].Pile[OKPile] = bus.busId;

            if ((cntEdge - 1) % 2) // 奇数-1
                edge[(cntEdge - 1) - 1].Pile[OKPile] = bus.busId;   // 双向边，两边一起处理
            else            // 偶数+1
                edge[(cntEdge - 1) + 1].Pile[OKPile] = bus.busId;
        }

        if (!addEdgeFlag) {
            bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
            edge[edgeId].Pile[OKPile] = bus.busId;

            if (edgeId % 2) // 奇数-1
                edge[edgeId - 1].Pile[OKPile] = bus.busId;   // 双向边，两边一起处理
            else            // 偶数+1
                edge[edgeId + 1].Pile[OKPile] = bus.busId;
        }

        curNode = edge[edgeId].from;

    }
}

// 先清空原来的业务对网络的影响
void reCoverNetwork(int lastBusID, int lastPileId) {

    ///////////////////////////////////////////////////////////////////////
    // 清空对寻路的影响
    vector<int> pathTmp = buses[lastBusID].pathTmp;
    int curNode = buses[lastBusID].end;
    while (pathTmp[curNode] != -1) {
        int edgeId = pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        edge[edgeId].Pile[lastPileId] = -1;
        --edge[edgeId].usedPileCnt;
        --node[curNode].passBusCnt;     // 经过节点的业务数减1

        if (edgeId % 2) { // 奇数-1
            edge[edgeId - 1].Pile[lastPileId] = -1;   // 双向边，两边一起处理
            --edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[lastPileId] = -1;
            --edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[pathTmp[curNode]].from;
    }

    --node[curNode].passBusCnt;     // 经过节点的业务数减1
    ////////////////////////////////////////////////////////////////////////
    // 清空对加放大器的影响
    buses[lastBusID].curA = D;
    curNode = buses[lastPileId].start;
    int trueNextEdgeId;
    for (int i = 0; i < buses[lastPileId].path.size(); ++i) {

        if (edge[buses[lastPileId].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[lastPileId].path[i] * 2;
        else
            trueNextEdgeId = buses[lastPileId].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[lastPileId].curA >= edge[trueNextEdgeId].trueD) {
            buses[lastPileId].curA -= edge[trueNextEdgeId].trueD;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[lastPileId].pileId] = -1;
            buses[lastBusID].curA = D;
            buses[lastBusID].curA -= edge[trueNextEdgeId].trueD;
        }
    }

    //////////////////////////////////////////////////////////////////////
    vector<int> nullVector, nullPath1, nullPath2;
    buses[lastBusID].mutiplierId.swap(nullVector);
    buses[lastBusID].path.swap(nullPath1);
    buses[lastBusID].pathTmp.swap(nullPath2);

    buses[lastBusID].pileId = -1;
    buses[lastBusID].curA = D;
}

// 重新加载业务到网络上
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp) {

    // 重新设置路径   
    int curNode = buses[lastBusID].end;
    buses[lastBusID].pileId = lastPileId;
    buses[lastBusID].pathTmp = vector<int>(pathTmp.begin(), pathTmp.end());
    while (buses[lastBusID].pathTmp[curNode] != -1) {
        int edgeId = buses[lastBusID].pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        ++node[curNode].passBusCnt;     // 经过节点的业务数加1

        buses[lastBusID].path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[lastPileId] = buses[lastBusID].busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // 奇数-1
            edge[edgeId - 1].Pile[lastPileId] = buses[lastBusID].busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeId + 1].Pile[lastPileId] = buses[lastBusID].busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[buses[lastBusID].pathTmp[curNode]].from;
    }
    ++node[curNode].passBusCnt;     // 经过节点的业务数加1
    reverseArray(buses[lastBusID].path);
    if (minPathSize[make_pair(buses[lastBusID].start, buses[lastBusID].end)] * goodBadGap > buses[lastBusID].path.size())
        buses[lastBusID].isBusWellAllocate = true;
    else
        buses[lastBusID].isBusWellAllocate = false;

    // 重新设置放大器
    addMultiplier(buses[lastBusID], lastBusID);

}

// 为分配好的业务添加放大器
void addMultiplier(Business& bus, int busId) {

    int curNode = buses[busId].start, trueNextEdgeId;
    for (int i = 0; i < buses[busId].path.size(); ++i) {

        if (edge[buses[busId].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[busId].path[i] * 2;
        else
            trueNextEdgeId = buses[busId].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[busId].curA >= edge[trueNextEdgeId].trueD) {
            buses[busId].curA -= edge[trueNextEdgeId].trueD;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[busId].pileId] = buses[busId].pileId;
            buses[busId].curA = D;
            buses[busId].curA -= edge[trueNextEdgeId].trueD;
            buses[busId].mutiplierId.push_back(edge[trueNextEdgeId].from);
        }
    }

}

int countEdgeNum(const vector<int>& path, unordered_set<int>& mark) {

    int edgeNum = 0;
    for (int i = 0, trueEdgeId; i < path.size(); ++i) {
        trueEdgeId = path[i] * 2;
        if (mark.find(trueEdgeId) != mark.end())      // 避免重复计数
            continue;
        else
            mark.emplace(trueEdgeId);
        ++edgeNum;
    }
    return edgeNum;
}

// 统计出path中的边的利用效率达到某一ratio的数目
int testEdgeFull(const vector<int>& path, unordered_set<int>& mark) {

    double ratio = 0.8;
    int n = path.size(), fullEdge = 0;
    for (int i = 0, trueEdgeId; i < n; ++i) {
        trueEdgeId = path[i] * 2;
        if (mark.find(trueEdgeId) != mark.end())      // 避免重复计数
            continue;
        else
            mark.emplace(trueEdgeId);
        if (edge[trueEdgeId].usedPileCnt > ratio * P)
            ++fullEdge;
    }
    return fullEdge;

}

// 复制网络
void copyNetwork() {

    for (int i = 0; i < T; ++i)
        copyBuses[i] = buses[i];
    copyNewEdge = newEdge;
    copyNewEdgePathId = newEdgePathId;
}
