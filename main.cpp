﻿//#include "generateRandomNet.h"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#define INF 2147483647

using namespace std;

int N, M, T, P, D; // 节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D
const int maxM = 50000; // 边的最大数目
const int maxN = 5000;  // 节点的最大数目
const int maxBus = 10000;   // 业务的最大数目
const int maxP = 80;    // 最大单边通道数量P

int cntEdge = 0;    // 当前边集数组所存储的边的数目
int cntBus = 0;     // 当前业务数组所存储业务的数目
int cntPlan = 0;    //输出方案的集合
class Node {
public:
    int NodeId;           // 实际上利用数组的下标就可以唯一标识Node，这里先做一个冗余
    int Multiplier[maxP]; // 该节点上存在的放大器，记录的是当前要放大的通道的编号，放大器不存在时值为-1
    vector<int> reachPile;  // 在单次dijkstra搜索中，可抵达该顶点的pile编号
}node[maxN];

class Edge {
public:
    int from, to, d, next;    // 起点，终点，边的距离，同起点的下一条边在edge中的编号
    int trueD;    // 边的真正距离
    int Pile[maxP]; // 该边上存在的通道，记录的是当前承载的业务的编号，不承载业务时值为-1
    Edge() {
        from = -1;
        to = -1;
        d = 0;
        next = -1;
    }
}edge[maxM],tempEdge[maxM];    // 边集数组

class Business {
public:
    int start;  // 业务起点
    int end;    // 业务终点
    int busId;  // 业务Id
    int curA;   // 当前信号强度
    Business() {
        start = -1;
        end = -1;
    }
    int pileId; // 业务所占据的通道Id
    vector<int> pathTmp;   // 存储从起点到其它点的最短路径的末边的编号（考虑通道堵塞的最短）
    vector<int> trueMinPath;   // 存储从起点到其它点的最短路径的末边的编号（不考虑通道堵塞的最短）
    vector<int> path;   // 存储路径所经过的边
    vector<int> mutiplierId;    // 存储所经过的放大器所在节点的编号
}buses[maxBus];

//用于写入方案数据的类
class Plan {
public:
    int planId;
    int edgeNum;
    vector<pair<int,int>> newAddEdge;//存储新加边的信息
    vector<vector<int>> taskAllocate;//存储业务分配的信息
    Plan() {
        planId = -1;
        edgeNum = -1;
    }
}plan[maxBus];


int head[maxN]; // head[i]，表示以i为起点的在逻辑上的第一条边在边集数组的位置（编号）
int dis[maxN];  // dis[i]，表示以源点到i到距离
bool vis1[maxN];  // 标识该点有无被访问过
bool vis2[maxN]; // 标识该点有无在添加某业务时，被路径搜索访问过
vector<pair<int, int>> newEdge; // 记录新添加的边的起点和终点
vector<int> newEdgePathId;   // 记录新边在边集中的位置（计数时，双向边视为同一边）
vector<int> eraseEdge;  // 记录下被清除掉的新边的编号

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

class Node1 {
public:
    int d, nodeId;

    Node1(int d, int nodeId) {
        this->d = d;
        this->nodeId = nodeId;
    }
    Node1() {}
    bool operator < (const Node1& x) const { // 重载运算符把最小的元素放在堆顶（大根堆）
        return d > x.d;
    }
};

priority_queue<Node1>q;  // 优先队列，存的是暂时的点nodeId到源点的距离和nodeId编号本身

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void dijkstra1(Business& bus);
void dijkstra2(Business& bus);
void dijkstra3(Business& bus);
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath);
bool dijkstra5(Business& bus, int blockEdge);
void BFS1(Business& bus);
void BFS2(Business& bus);
bool BFS5(Business& bus, int blockEdge);
void loadBus(int busId);
void allocateBus();
void reAllocateBus();
void reverseArray(vector<int>& arr);
void outPut();
void outPut01();
bool bfsTestConnection(int start, int end);
void findAddPath(Business& bus, bool* vis2);
void reCoverNetwork(int lastBusID, int lastPileId);
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);
void addPlan();
bool dijkstra00(Business& bus);
bool loadBus00(int busId);
vector<bool> generatePlan(vector<pair<int, int>>& planSet);
bool existPlan(int maxCost, int minCost);
void outPut01();
bool SA();//模拟退火算法寻优
int costFunction(Plan& p);
int testTime = 0;
int maxTestTime = 10;

// 主函数
int main() {

    cin >> N >> M >> T >> P >> D;
    init();
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        cin >> s >> t >> d;
        minDist[make_pair(s, t)] = INF;
        minDist[make_pair(t, s)] = INF;
        addEdge(s, t, d);
        addEdge(t, s, d);   // 添加双向边
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        cin >> Sj >> Tj;
        addBus(Sj, Tj); // 添加业务
    }

    allocateBus();
    reAllocateBus();
  /*  addPlan();
    outPut01();*/
    outPut();
    return 0;
}

// 将所有的业务分配到光网络中
void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i);
        if (i % 100 == 99)
            reAllocateBus();
    }
}

// 试图重新分配业务到光网络中
void reAllocateBus() {

    int n = newEdge.size(), trueEdgeId;
    for (int idx = 0; idx < n; ++idx) {
        //for (int i = n - 1; i >= 0; --i) {
        int i = newEdgePathId[idx]; // i为边在边集数组的编号（计数时，双向边视作同一边）

        trueEdgeId = i * 2;
        int busCnt = 0;
        vector<int> lastBusIds, lastPileIds;

        for (int j = 0; j < P; ++j)
            if (edge[trueEdgeId].Pile[j] != -1 && edge[trueEdgeId].Pile[j] != T) {   // 说明在通道j上承载了该业务
                ++busCnt;
                lastBusIds.push_back(edge[trueEdgeId].Pile[j]);
                lastPileIds.push_back(j);
            }

        bool findPath = false;

        if (busCnt == 0) {      // 如果该新边上，一条业务都没有承载，直接删边

            for (int k = 0; k < newEdge.size(); ++k) {

                if (newEdgePathId[k] == i) {        // 删去对应边
                    newEdge.erase(newEdge.begin() + k);
                    newEdgePathId.erase(newEdgePathId.begin() + k);
                    --idx;
                    --n;
                    break;
                }

            }
            for (int k = 0; k < P; ++k) {   // 该边已删除，就应对其进行封锁
                edge[trueEdgeId].Pile[k] = T;
                edge[trueEdgeId + 1].Pile[k] = T;   // 偶数+1
            }
            //eraseEdge.push_back(i + M);     // 记下删边的编号
        }
        else {  // 如果该新边上，承载了多条业务，则对该边上的所有业务重新分配，看能否不依赖该新边

            vector<vector<int>> pathTmp(busCnt, vector<int>());     // 用于此后重新加载边
            for (int k = 0; k < busCnt; ++k) {
                pathTmp[k] = buses[lastBusIds[k]].pathTmp;
                reCoverNetwork(lastBusIds[k], lastPileIds[k]);
            }

            int stopK = -1;
            vector<int> tmpLastPileIds;
            for (int k = 0; k < busCnt; ++k) {
                //findPath = dijkstra5(buses[lastBusIds[k]], i);
                findPath = BFS5(buses[lastBusIds[k]], i);
                if (findPath == false) {
                    stopK = k;
                    break;
                }
                tmpLastPileIds.push_back(buses[lastBusIds[k]].pileId);   // 原本的pileId已改变，此处进行更新，以防止reCoverNetwork时出bug
            }

            if (findPath) {

                for (int k = 0; k < newEdge.size(); ++k) {

                    if (newEdgePathId[k] == i) {    // 删去对应边
                        newEdge.erase(newEdge.begin() + k);
                        newEdgePathId.erase(newEdgePathId.begin() + k);
                        --idx;
                        --n;
                        break;
                    }

                }

                for (int k = 0; k < P; ++k) {   // 该边已删除，就应对其进行封锁
                    edge[trueEdgeId].Pile[k] = T;
                    edge[trueEdgeId + 1].Pile[k] = T;   // 偶数+1
                }

                //eraseEdge.push_back(i + M);     // 记下删边的编号

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
void loadBus(int busId) {
    //dijkstra1(buses[busId]);
    BFS1(buses[busId]);
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

    head[s] = cntEdge++;    // 更新以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
}
void addTempEdge(int s, int t, int d,int index) {
    tempEdge[index].from = s; // 起点
    tempEdge[index].to = t;   // 终点
    tempEdge[index].d = 1;    // 距离
    //edge[cntEdge].d = d;    // 距离
    tempEdge[index].trueD = d;    // 距离
    tempEdge[index].next = head[s];   // 链式前向。以s为起点下一条边的编号，head[s]代表的是当前以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    for (int i = 0; i < P; ++i)
        tempEdge[index].Pile[i] = -1;

    head[s] = index++;    // 更新以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
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

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    // 用于优化单个业务的加边策略，但全局来看效果不好
    for (int i = 0; i < N; ++i) {
        vis2[i] = false;
        vector<int>().swap(node[i].reachPile);
    }
    vis2[start] = true;

    for (; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            dis[i] = INF;
            vis1[i] = false;
        }
        dis[start] = 0;  // 源点到源点的距离为0
        priority_queue<Node1> null_queue; // 定义一个空的priority_queue对象
        q.swap(null_queue);
        q.push(Node1(0, start));
        int s = -1;
        while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
            Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
            q.pop();
            s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

            if (s == end) { // 当end已经加入到了生成树，则结束搜索
                break;
                bus.pileId = p;
            }

            // 没有遍历过才需要遍历
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    vis2[t] = true;
                    node[t].reachPile.push_back(p);

                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                        dis[t] = dis[s] + edge[i].d;   // 松弛操作
                        q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中
                    }
                }

            }
        }
        if (s == end) { // 当end已经加入到了生成树，则结束搜索

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // 找不到路，需要构造新边，以下提供了两种策略，使用时需要注释掉其中一个
        //findAddPath(bus, vis2); // 用于优化单个业务的加边策略，但全局来看效果不好
        dijkstra2(bus);       // 旧的加边策略，一但加边，整个路径都会加，但全局性能是当前最好的
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;

        if (edgeId % 2) // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
        else            // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;


        curNode = edge[bus.pathTmp[curNode]].from;
    }
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

    priority_queue<Node1> null_queue; // 定义一个空的priority_queue对象
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

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // 源点到源点的距离为0

    priority_queue<Node1> null_queue; // 定义一个空的priority_queue对象
    q.swap(null_queue);// 交换队列容器中的内容
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
        priority_queue<Node1> null_queue; // 定义一个空的priority_queue对象
        q.swap(null_queue);
        q.push(Node1(0, start));
        int s = -1;
        while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
            Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
            q.pop();
            s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

            if (s == end) { // 当end已经加入到了生成树，则结束搜索
                break;
                bus.pileId = p;
            }

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

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
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

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;

        if (edgeId % 2) // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
        else            // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;


        curNode = edge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);
    return true;

}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void BFS1(Business& bus) {

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

        while (!bfsQ.empty() && !getOutFlag) { // 队列为空即，所有点都被加入到生成树中去了

            s = bfsQ.front().first;
            curLevel = bfsQ.front().second;
            bfsQ.pop();

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
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (s == end) {

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // 找不到路，需要构造新边
        BFS2(bus);       // 旧的加边策略，一但加边，整个路径都会加，但全局性能是当前最好的
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;

        if (edgeId % 2) // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
        else            // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);
    if (bus.busId == 6)
        int a = 1;

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

    while (!bfsQ.empty() && !getOutFlag) { // 队列为空即，所有点都被加入到生成树中去了

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
    BFS1(bus);

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
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (s == end) {
            
            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
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

        if (edgeId % 2) // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
        else            // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);
    return true;
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

        if (edgeId % 2) // 奇数-1
            edge[edgeId - 1].Pile[lastPileId] = -1;   // 双向边，两边一起处理
        else            // 偶数+1
            edge[edgeId + 1].Pile[lastPileId] = -1;

        curNode = edge[pathTmp[curNode]].from;
    }

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

        buses[lastBusID].path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[lastPileId] = buses[lastBusID].busId;

        if (edgeId % 2) // 奇数-1
            edge[edgeId - 1].Pile[lastPileId] = buses[lastBusID].busId;   // 双向边，两边一起处理
        else            // 偶数+1
            edge[edgeId + 1].Pile[lastPileId] = buses[lastBusID].busId;

        curNode = edge[buses[lastBusID].pathTmp[curNode]].from;
    }
    reverseArray(buses[lastBusID].path);

    // 重新设置放大器
    curNode = buses[lastBusID].start;
    int trueNextEdgeId;
    for (int i = 0; i < buses[lastBusID].path.size(); ++i) {

        if (edge[buses[lastBusID].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[lastBusID].path[i] * 2;
        else
            trueNextEdgeId = buses[lastBusID].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[lastBusID].curA >= edge[trueNextEdgeId].trueD) {
            buses[lastBusID].curA -= edge[trueNextEdgeId].trueD;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[lastBusID].pileId] = buses[lastBusID].pileId;
            buses[lastBusID].curA = D;
            buses[lastBusID].curA -= edge[trueNextEdgeId].trueD;
            buses[lastBusID].mutiplierId.push_back(edge[trueNextEdgeId].from);
        }
    }
}
void outPut01() {
    
    vector<int> cost;
    for (int i = 0; i < cntPlan; i++) {
        cost[i] = costFunction(plan[i]) ;
    }
    
    //寻找成本最低的方案
    int index=0;
    int flag=0;
    for (int j = 0; j < cntPlan-1; j++) {
        if (plan[j].planId != -1 && plan[j + 1].planId != -1) {
            if (cost[j] < cost[j + 1]) {
                index = j;
            }
            else {
                index = j + 1;
            }
            flag++;//判定接受的方案个数
        }
       
    }
    if (flag == 0) {//如果接受的方案为0，则输出初始值
        cout << plan[0].edgeNum;
        cout << endl;
        for (int i = 0; i < plan[0].edgeNum; i++) {
            cout << plan[0].newAddEdge[i].first << " " << plan[0].newAddEdge[i].second << " ";
            cout << endl;
        }

        for (int j = 0; j < T; j++) {
            vector<int> temp(plan[0].taskAllocate[j]);
            for (int k = 0; k < plan[0].taskAllocate[j].size(); k++) {
                cout << temp[k] << " ";
            }
            cout << endl;
        }
        return;
    }

    cout << plan[index].edgeNum;
    cout << endl;
    for (int i = 0; i < plan[index].edgeNum; i++) {
        cout << plan[index].newAddEdge[i].first << " " << plan[index].newAddEdge[i].second << " ";
        cout << endl;
    }

    for (int j = 0; j < T; j++) {
        vector<int> temp(plan[index].taskAllocate[j]);
        for (int k = 0; k < plan[index].taskAllocate[j].size(); k++) {
            cout << temp[k] << " ";
        }
        cout << endl;
    }
    

}
int costFunction(Plan& p) {
    //输出总成本，int的表示范围为-2^31---2^31-1;
    int result= p.edgeNum * pow(10, 6);
    for (int i = 0; i < T; i++) {
        vector<int> temp;
        temp = p.taskAllocate[i];
        result = result + temp[2]*100;
    }
    return result;

}
// 把业务busId加载到光网络中
bool loadBus00(int busId) {
    //dijkstra1(buses[busId]);
   bool flag= dijkstra00(buses[busId]);
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
    return flag;
}
bool dijkstra00(Business& bus) {
    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    // 用于优化单个业务的加边策略，但全局来看效果不好
    for (int i = 0; i < N; ++i) {
        vis2[i] = false;
        vector<int>().swap(node[i].reachPile);
    }
    vis2[start] = true;

    for (; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            dis[i] = INF;
            vis1[i] = false;
        }
        dis[start] = 0;  // 源点到源点的距离为0
        priority_queue<Node1> null_queue; // 定义一个空的priority_queue对象
        q.swap(null_queue);
        q.push(Node1(0, start));
        int s = -1;
        while (!q.empty()) {   // 堆为空即，所有点都被加入到生成树中去了
            Node1 x = q.top();  // 记录堆顶（堆内最小的边）并将其弹出
            q.pop();
            s = x.nodeId;   // 点s是dijstra生成树上的点，源点到s的最短距离已确定

            if (s == end) { // 当end已经加入到了生成树，则结束搜索
                break;
                bus.pileId = p;
            }

            // 没有遍历过才需要遍历
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i =tempEdge[i].next) { // 搜索堆顶所有连边

                if (tempEdge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = tempEdge[i].to;
                    vis2[t] = true;
                    node[t].reachPile.push_back(p);

                    if (dis[t] > dis[s] + tempEdge[i].d) {
                        tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                        dis[t] = dis[s] + tempEdge[i].d;   // 松弛操作
                        q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中
                    }
                }

            }
        }
        if (s == end) { // 当end已经加入到了生成树，则结束搜索

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
                curNode = tempEdge[edgeId].from;
                tmpDist += tempEdge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // 找不到路，需要构造新边，以下提供了两种策略，使用时需要注释掉其中一个
        
        return false;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id

        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        tempEdge[edgeId].Pile[choosenP] = bus.busId;

        if (edgeId % 2) // 奇数-1
            tempEdge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
        else            // 偶数+1
            tempEdge[edgeId + 1].Pile[choosenP] = bus.busId;


        curNode = tempEdge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);
    return true;
}
//随机删除一条边，返回各方案是否可行的标志位
vector<bool> deleteOneEdge(int edgeNum,int amplifierNum) {
    vector<bool> result;
  for (int j = 0; j < M; j++) {
        tempEdge[j] = edge[j];
  }
  int flag=0;
  while (flag != newEdge.size()) {
       for (int k = M; k < M + newEdge.size() - 1&&(k!=flag); k++) {
             addTempEdge(newEdge[k - M].first, newEdge[k - M].second, minDist[make_pair(newEdge[k- M].first, newEdge[k- M].second)], k);//主要思路，少加一条边
       }
       vector<bool> loadCondition;
       for (int kk = 0; kk < T; kk++) {
          loadCondition.push_back( loadBus00(buses[kk].busId));
       }
       addPlan();
       for (int kkk = 0; kkk < loadCondition.size(); kkk++) {
           if (loadCondition[kkk] == false) {
               result.push_back(false);//只要有一个任务分配不成功，就证明该方案不可行
               plan[cntPlan - 1].planId = -1;//-1代表该方案不可被接受
               continue;
           }
       }
       //判定放大器的个数，是否合乎要求
       int curApliphierNum=0;
       for (int i = 0; i < T; i++) {
           curApliphierNum += buses[i].mutiplierId.size();
       }
       if (curApliphierNum <= amplifierNum) {
           result.push_back(true);
       }
       result.push_back(false);
       plan[cntPlan - 1].planId = -1;
       flag++;
  }
  return result;
}
//随机删除两条边
vector<bool> deleteTwoEdge(int edgeNum, int amplifierNum) {
    vector<bool> result;
    for (int j = 0; j < M; j++) {
        tempEdge[j] = edge[j];
    }
    //随机减两条边
    int flag1 = 0;
    while (flag1 != newEdge.size()) {
        int flag2 = 0;
        while (flag2 != newEdge.size()) {
            for (int k = M; k < M + newEdge.size() - 2 && (k != flag1)&&(flag1!=flag2); k++) {
                addTempEdge(newEdge[k - M].first, newEdge[k - M].second, minDist[make_pair(newEdge[k - M].first, newEdge[k - M].second)], k);//

            }
            vector<bool> loadCondition;
            for (int kk = 0; kk < T; kk++) {
                loadCondition.push_back(loadBus00(buses[kk].busId));
            }
            addPlan();
            for (int kkk = 0; kkk < loadCondition.size(); kkk++) {
                if (loadCondition[kkk] == false) {
                    result.push_back(false);//只要有一个任务分配不成功，就证明该方案不可行
                    plan[cntPlan - 1].planId = -1;//-1代表该方案不可被接受
                    continue;
                }
            }
            //判定放大器的个数，是否合乎要求
            int curApliphierNum=0;
            for (int i = 0; i < T; i++) {
                curApliphierNum += buses[i].mutiplierId.size();
            }
            if (curApliphierNum <= amplifierNum) {
                result.push_back(true);
            }
            result.push_back(false);
            plan[cntPlan - 1].planId = -1;
            flag2++;
        }
        
        flag1++;
    }
    return result;
}
//随机删除三条边
vector<bool> deleteThreeEdge(int edgeNum, int amplifierNum) {
    vector<bool> result;
    for (int j = 0; j < M; j++) {
        tempEdge[j] = edge[j];
    }
    //随机减两条边
    int flag1 = 0;
    while (flag1 != newEdge.size()) {
        int flag2 = 0;
        while (flag2 != newEdge.size()) {
            int flag3 = 0;
            while (flag3 != newEdge.size()) {
                if (flag1 != flag2 && flag2 != flag3 && flag1 != flag3) {
                    for (int k = M; k < M + newEdge.size() - 3&& (k != flag1) && (flag1 != flag2); k++) {
                        addTempEdge(newEdge[k - M].first, newEdge[k - M].second, minDist[make_pair(newEdge[k - M].first, newEdge[k - M].second)], k);//
                    }
                    vector<bool> loadCondition;
                    for (int kk = 0; kk < T; kk++) {
                        loadCondition.push_back(loadBus00(buses[kk].busId));
                    }
                    addPlan();
                    for (int kkk = 0; kkk < loadCondition.size(); kkk++) {
                        if (loadCondition[kkk] == false) {
                            result.push_back(false);//只要有一个任务分配不成功，就证明该方案不可行
                            plan[cntPlan - 1].planId = -1;//-1代表该方案不可被接受
                            continue;
                        }
                    }
                    //判定放大器的个数，是否合乎要求
                    int curApliphierNum=0;
                    for (int i = 0; i < T; i++) {
                        curApliphierNum += buses[i].mutiplierId.size();
                    }
                    if (curApliphierNum <= amplifierNum) {
                        result.push_back(true);
                    }
                    result.push_back(false);
                    plan[cntPlan - 1].planId = -1;
                }
                flag3++;
            }
            
            flag2++;
        }

        flag1++;
    }
    return result;
}
void addPlan() {
    plan[cntPlan].planId = cntPlan;

    vector<pair<int, int>> tempVec1;//存储临时的边的信息
    vector<vector<int>> tempVec2;//存储全部任务的信息

    tempVec1.swap(newEdge);
    plan[cntPlan].newAddEdge = tempVec1;
    for (int i = 0; i < T; ++i) {
        vector<int> tempVec3;//存储单个任务的信息
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();
        tempVec3.push_back(buses[i].pileId);
        tempVec3.push_back(pSize);
        tempVec3.push_back(mSize);

        for (int j = 0; j < pSize; j++) {
            tempVec3.push_back(buses[i].path[j]);
        }

        for (int k = 0; k < mSize; k++) {
            tempVec3.push_back(buses[i].mutiplierId[k]);
        }

        tempVec2.push_back(tempVec3);
    }
    plan[cntPlan].taskAllocate = tempVec2;
    cntPlan++;
}
vector<bool> generatePlan(vector<pair<int,int>>& planSet) {
//核心代码，根据上一次迭代的结果生成随机的子方案
    //思路：首先根据加边数量随机减边，根据上一次生成的结果随机减一条边，然后根据减的边的数量去分配放大器和通道
    //观察是否能分配，如果能，则返回true，并将成功的结果方案写入plan中，若不能则返回false，该方案将被否定同时其id将会被写为-1
    //需要在一个界内去搜索最佳答案，比如说在6.3KW-6.1KW的范围中，去搜寻可能的方案，比如说我减两条边，固定了成本
    //再判断其是否能在当前成本的区域内生成解决方案
    //在两百万的下降区间内，可能存在的方案是，减一条边，加1W个放大器，减两条边，关键是通道的分配方案
    vector<bool> result;
    for (int i = 0; i < planSet.size(); i++) {
        int edgeNum = planSet[i].first;
        int amplifierNum = planSet[i].second;
        if (edgeNum > 0&&edgeNum<=newEdge.size()) {
            switch (edgeNum) {
            case(1):
            {
              vector<bool> vec = deleteOneEdge(edgeNum, amplifierNum);
                break;
            }
            case(2): {
                vector<bool> vec = deleteTwoEdge(edgeNum, amplifierNum);
                break;
            } 
            case(3):
            {
                vector<bool> vec = deleteThreeEdge(edgeNum, amplifierNum);
                break;
            }
              
            }
        }
        else {
            result.push_back(false);
        }     
    }

    return result;
}

bool existPlan(int maxCost,int minCost) {
    //在价格区间内搜寻适配的方案
    //在给定的价格区间，我们可以获得，可能存在的放大器与加边数的集合，然后测试在当前条件下，是否存在方案符合条件，
    // 因为每个放大器只能放大一条业务，而每个业务会占用每条边相同编号的通道，如果不改变其分配的通道，那么其必定是需要加放大器的，因此业务分配的关键在于通道的分配，
    // 只能通过调整通道的分配，来减少加边，不能从宏观上直接减边来降低成本，关键还是在于任务通道的分配，
    // 两个节点可能存在多条边，边的使用顺序也很重要，可能会直接影响到加边的成本
    // 可通过业务分配的顺序来优化成本
    // 两个优化方向：业务的分配顺序，通道的分配
    //执行删边策略
    int maxEdgeNum = (maxCost - minCost) / pow(10, 6);
    int maxAmplifierNum = maxEdgeNum * pow(10, 4);
    vector<pair<int, int>> planSet;//可能存在的方案集合
    for (int i = maxEdgeNum; i > 0; i--) {
        planSet.push_back(make_pair(i, maxAmplifierNum - i * pow(10, 4)));
    }
   vector<bool> result= generatePlan(planSet);
   for (int i = 0; i < result.size(); i++) {
       if (result[i] == true) {
           return true;
       }
   }
}
bool SA() {
    /*模拟退火的流程：在任一温度水平下，随机扰动产生新解，并计算目标函数值的变化，退火应该时退成本，决定值是一个成本的区间，
    求得是一个最优成本的区间，每次随机降一个成本的区间，搜索的是在该区间存不存在方案，使得输出条件成立
    决定是否被接受，由于算法初始温度较高，这样，使E增大的新解在初始时也能被接受*/
    /*算法编写思路：算法生成的随机值应该是一个随机的价格区间，然后在价格区间内判断是否存在方案，
    使得退火算法收敛到一个可行的成本方案，通过调整接受概率，来调整参数来控制每次迭代的时间复杂度，和收敛速度
    */

    int K = 0.98;//K越大接受概率就小，收敛的更快
    int inner_max_iter = 200;//最大内循环迭代次数，主要更新温度值
    int out_max_iter = 10;//最大外循环迭代次数，主要更新温度衰减率
    int originCost = costFunction(plan[0]);//初始值
    int l = 1;
    while (l < inner_max_iter) {//内循环，寻找一定温度下的最优值
        int randEdge = rand() % 3 + 1;//生成1-3之间的随机数
        int randSymbol = pow(-1, rand());//随机符号与正负
        randEdge *= randSymbol;
        if (originCost - randEdge * pow(10, 6) < originCost) {
            if (existPlan(originCost, originCost - randEdge * pow(10, 6))) {
                int de = costFunction(plan[cntPlan]) - costFunction(plan[cntPlan - 1]);
                if (de < 0) {
                    plan[cntPlan].planId = 1;//接受当前的结果为最优解
                    plan[cntPlan - 1].planId = -1;
                }
                else {
                    double rd = rand() / (RAND_MAX + 1);//随机生成的接受概率,小于1
                    if (exp(de / originCost > rd && exp(de / originCost) < 1)) {
                        plan[cntPlan].planId = 1;//表示舍弃上一次的结果，接受当前的结果为最优解
                        plan[cntPlan - 1].planId = -1;
                    }
                }
            }
            else {
                l++;
                continue;
            }
           
        }
        else {
            if (existPlan(originCost - randEdge * pow(10, 6), originCost)) {
                int de = costFunction(plan[cntPlan]) - costFunction(plan[cntPlan - 1]);
                if (de < 0) {
                    plan[cntPlan].planId = 1;//接受当前的结果为最优解
                   // plan[cntPlan - 1].planId = -1;
                }
                else {
                    double rd = rand() / (RAND_MAX + 1);//随机生成的接受概率,小于1
                    if (exp(de / originCost > rd && exp(de / originCost) < 1)) {
                        plan[cntPlan].planId = 1;//表示舍弃上一次的结果，接受当前的结果为最优解
                       // plan[cntPlan - 1].planId = -1;
                    }
                }
            }
            else {
                l++;
                continue;
            }
         
        }
        l++;
        originCost *= K;
    }

    return true;
}