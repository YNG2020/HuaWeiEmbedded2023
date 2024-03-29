#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <math.h>
#include <time.h>
#include <cmath>
#include <fstream>
#include "struct.h"
#include "configure.h"
const int INF = 2147483647;

using namespace std;

int N, M, T, P, D; // 节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D
int cntEdge = 0;    // 当前边集数组所存储的边的数目
int cntBus = 0;     // 当前业务数组所存储业务的数目

int head[configure::maxN]; // head[i]，表示以i为起点的在逻辑上的第一条边在边集数组的位置（编号）
bool vis1[configure::maxN];  // 标识该点有无被访问过
vector<pair<int, int>> newEdge; // 记录新添加的边的起点和终点
vector<int> newEdgePathId;   // 记录新边在边集中的位置（计数时，双向边视为同一边）
vector<int> remainBus;  // 记录下初次分配时，因路径堵塞而无法分配边的业务的编号

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void BFS_loadBus(Business& bus, bool ifLoadNewEdge);
bool BFS_detectPath(Business& bus, int blockEdge);
void BFS_addNewEdge(Business& bus);
void loadBus(int busId, bool ifLoadRemain);
void loadMultiplier(int busId);
void allocateBus();
void reAllocateBus(int HLim);
void tryDeleteEdge();
void reverseArray(vector<int>& arr);
void outPut();
void reCoverNetwork(int lastBusID, int lastPileId);
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);

bool ifLast = false;
bool ifTryDeleteEdge = true;

// 主函数
int main() {

    std::ifstream cin("dataMATLAB.txt"); // 打开文件
    if (!cin) {
        std::cerr << "Error: Cannot open the file." << std::endl;
        return 1;
    }

    cin >> N >> M >> T >> P >> D;
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
    }

    allocateBus();

    ifTryDeleteEdge = false;

    int cnt = 0;

    if (!configure::isUseProblemFinder)
        std::cout << "Original newEdge.size" << newEdge.size() << endl;
    while (cnt < configure::cntLimit) {

        reAllocateBus(pow(2.71, -0.005 * cnt)*T);
        tryDeleteEdge();
        if (!configure::isUseProblemFinder)
            std::cout << "newEdge.size" << newEdge.size() << endl;
        cnt = cnt + 1;
    }

    ifLast = true;
    tryDeleteEdge();
    tryDeleteEdge();
    if (!configure::isUseProblemFinder)
        std::cout << "newEdge.size" << newEdge.size() << endl;
    outPut();

    return 0;
}

// 初始化
void init() {

    for (int i = 0; i < N; ++i) {

        head[i] = -1;
        node[i].NodeId = i;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }

    for (int i = 0; i < T; ++i) {
        buses[i].curA = D;
    }

}

// 将所有的业务分配到光网络中
void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i, false);
    }
}

// 试图重新分配业务到光网络中
void reAllocateBus(int HLim) {

    int gap = max(int(0.025 * T), 20);
    if (gap > T)
        return;
    vector<int> totBusIdx(T, 0);
    vector<int> busIdx(gap, 0);
    for (int i = 0; i < T; ++i)
        totBusIdx[i] = i;

    for (int i = 0; i + gap < HLim; i = i + gap) {

        srand(1);  // 设置随机数种子  
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
        vector<vector<int>> pathTmp2(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busId; j >= i; --j) {
            busId = busIdx[j - i];
            loadBus(busId, false);
            pathTmp2[j - i] = buses[busId].pathTmp;
            pileTmp2[j - i] = buses[busId].pileId;
            curEdgeNum += buses[busId].path.size();
        }

        if (oriEdgeNum > curEdgeNum) {  // 总体的边数减少，接受迁移
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
        int busCnt = 0;
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

                findPath = BFS_detectPath(buses[lastBusIds[k]], idxEdge);

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

                    int curNode = buses[lastBusIds[k]].start, trueNextEdgeId;
                    for (int i = 0; i < buses[lastBusIds[k]].path.size(); ++i) {

                        if (edge[buses[lastBusIds[k]].path[i] * 2].from == curNode)
                            trueNextEdgeId = buses[lastBusIds[k]].path[i] * 2;
                        else
                            trueNextEdgeId = buses[lastBusIds[k]].path[i] * 2 + 1;
                        curNode = edge[trueNextEdgeId].to;

                        if (buses[lastBusIds[k]].curA >= edge[trueNextEdgeId].trueD) {
                            buses[lastBusIds[k]].curA -= edge[trueNextEdgeId].trueD;
                        }
                        else {
                            node[edge[trueNextEdgeId].from].Multiplier[buses[lastBusIds[k]].pileId] = buses[lastBusIds[k]].pileId;
                            buses[lastBusIds[k]].curA = D;
                            buses[lastBusIds[k]].curA -= edge[trueNextEdgeId].trueD;
                            buses[lastBusIds[k]].mutiplierId.push_back(edge[trueNextEdgeId].from);
                        }
                    }

                }

            }
            else {

                for (int k
                    = 0; k < stopK; ++k) {   // 把试图寻路时，造成的对网络的影响消除
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

    BFS_loadBus(buses[busId], false);
    loadMultiplier(busId);

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

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void BFS_loadBus(Business& bus, bool ifLoadNewEdge) {

    int start = bus.start, end = bus.end;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    static int addNewBus = 0;   // 加业务次数
    ++addNewBus;
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    double maxValue = -1;

    for (int p = 0; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        vis1[start] = true;
        int s = start;
        int t = -1;
        int curLevel = 0;

        while (!bfsQ.empty() && t != end) { // 队列为空即，所有点都被加入到生成树中去了

            s = bfsQ.front().first;
            curLevel = bfsQ.front().second;
            bfsQ.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

                    if (t == end) {
                        ++curLevel;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (t == end) {

            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // 键不存在
                minPathSize[make_pair(start, end)] = curLevel;
            else if (minPathSize[make_pair(start, end)] > curLevel) {
                minPathSize[make_pair(start, end)] = curLevel;
            }

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
    if (findPath == false) {    // 找不到路，需要构造新边

        if (ifTryDeleteEdge) {
            if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))   // 抑制过度删边（删边需时较长）
                tryDeleteEdge();
        }
        BFS_addNewEdge(bus);       // 新加边策略，只加最短路径上需要进行加边的边
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id

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

        curNode = edge[edgeId].from;
    }
    reverseArray(bus.path);
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回
bool BFS_detectPath(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (int p = 0; p < P; ++p) {
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

        while (!bfsQ.empty() && !getOutFlag) { // 队列为空即，所有点都被加入到生成树中去了

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
    reverseArray(bus.path);
    return true;
}

// 不能加载业务时，寻找业务bus的起点到终点的路径（不考虑通道堵塞，全通道搜索），并对路径上的发生堵塞的边进行加边处理
void BFS_addNewEdge(Business& bus) {

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

    ofstream cout("result.txt");
    if (!cout.is_open()) {
        cout << "Error opening file." << endl;
    }
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
    reverseArray(buses[lastBusID].path);

    // 重新设置放大器
    loadMultiplier(lastBusID);
}

// 加载放大器到业务上
void loadMultiplier(int busId) {
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