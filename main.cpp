#include "generateRandomNet.h"
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
    }
    int pileId; // 业务所占据的通道Id
    vector<int> pathTmp;   // 存储从起点到其它点的最短路径的末边的编号（考虑通道堵塞的最短）
    vector<int> trueMinPath;   // 存储从起点到其它点的最短路径的末边的编号（不考虑通道堵塞的最短）
    vector<int> path;   // 存储路径所经过的边
    vector<int> mutiplierId;    // 存储所经过的放大器所在节点的编号
}buses[maxBus];

int head[maxN]; // head[i]，表示以i为起点的在逻辑上的第一条边在边集数组的位置（编号）
int dis[maxN];  // dis[i]，表示以源点到i到距离
bool vis1[maxN];  // 标识该点有无被访问过
bool vis2[maxN]; // 标识该点有无在添加某业务时，被路径搜索访问过
int minDist2start[maxN]; // 记录该点到起点在考虑通道堵塞下的（多通道中的最小）距离，值为-1时，该点不可达，值为0或更大时，值代表该点到起点的边数
int tmpDist2start[maxN]; // 记录该点到起点在考虑通道堵塞下的（当前通道中的）距离，值为-1时，该点不可达，值为0或更大时，值代表该点到起点的边数vector<pair<int, int>> newEdge; // 记录新添加的边的起点和终点
vector<pair<int, int>> newEdge; // 记录新添加的边的起点和终点

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
queue<pair<int, int>> bfsQ;

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void dijkstra1(Business& bus);
void dijkstra2(Business& bus);
void dijkstra3(Business& bus);
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath);
void BFS1(Business& bus);
void BFS2(Business& bus);
void BFS3(Business& bus);
void BFS4(int start, int end, int pileId, vector<int>& tmpOKPath);
void loadBus(int busId);
void allocateBus();
void reverseArray(vector<int>& arr);
void outPut();
bool bfsTestConnection(int start, int end);
void findAddPath1(Business& bus, bool* vis2);
void findAddPath2(Business& bus, int* vis2);

int testTime = 0;
int maxTestTime = 10;
vector<vector<int>> rEdge;
vector<vector<int>> rBus;
vector<int> allNewEdgeCnt;
vector<int> allPathCnt;
vector<int> allMultiplierCnt;

//// 主函数
//int main() {
//
//    cin >> N >> M >> T >> P >> D;
//    init();
//    int s = 0, t = 0, d = 0;
//    for (int i = 0; i < M; ++i) {
//        cin >> s >> t >> d;
//        minDist[make_pair(s, t)] = INF;
//        minDist[make_pair(t, s)] = INF;
//        addEdge(s, t, d);
//        addEdge(t, s, d);   // 添加双向边
//    }
//
//    int Sj, Tj;
//    for (int i = 0; i < T; ++i) {
//        cin >> Sj >> Tj;
//        addBus(Sj, Tj); // 添加业务
//    }
//
//    allocateBus();
//    outPut();
//
//    return 0;
//}


// 主函数
int main() {

    allNewEdgeCnt.resize(maxTestTime, 0);
    allPathCnt.resize(maxTestTime, 0);
    allMultiplierCnt.resize(maxTestTime, 0);

    for (; testTime < maxTestTime; ++testTime) {
        cntEdge = 0;    // 当前边集数组所存储的边的数目
        cntBus = 0;     // 当前业务数组所存储业务的数目
        minDist.clear();    // 清空
        vector<vector<int>>().swap(rEdge);
        vector<vector<int>>().swap(rBus);
        vector<pair<int, int>>().swap(newEdge);

        generateRandomNet(rEdge, rBus, N, M, T, P, D);

        init();
        int s = 0, t = 0, d = 0;
        for (int i = 0; i < M; ++i) {
            s = rEdge[i][0], t = rEdge[i][1], d = rEdge[i][2];
            minDist[make_pair(s, t)] = INF;
            minDist[make_pair(t, s)] = INF;
            addEdge(s, t, d);
            addEdge(t, s, d);   // 添加双向边
        }

        int Sj, Tj;
        for (int i = 0; i < T; ++i) {
            Sj = rBus[i][0], Tj = rBus[i][1];
            addBus(Sj, Tj); // 添加业务
        }

        allocateBus();
        outPut();
        cout << "\ntestTime " << testTime << endl;
        cout << "allNewEdgeCnt " << allNewEdgeCnt[testTime] << endl;
        cout << "allPathCnt " << allPathCnt[testTime] << endl;
        cout << "allMultiplierCnt " << allMultiplierCnt[testTime] << endl;
    }

    return 0;
}

void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i);
    }
}

void loadBus(int busId) {
    //BFS1(buses[busId]);
    dijkstra1(buses[busId]);
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

void init() {   // 初始化

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

void addEdge(int s, int t, int d) {    // 加边函数，s起点，t终点，d距离
    edge[cntEdge].from = s; // 起点
    edge[cntEdge].to = t;   // 终点
    edge[cntEdge].d = 1;    // 距离
    edge[cntEdge].trueD = d;    // 距离
    edge[cntEdge].next = head[s];   // 链式前向。以s为起点下一条边的编号，head[s]代表的是当前以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    for (int i = 0; i < P; ++i)
        edge[cntEdge].Pile[i] = -1;

    head[s] = cntEdge++;    // 更新以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
}

void addBus(int start, int end) {   // 加业务函数
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    vector<int>().swap(buses[cntBus].path);
    vector<int>().swap(buses[cntBus].pathTmp);
    vector<int>().swap(buses[cntBus].mutiplierId);
    
    ++cntBus;
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void BFS1(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    // 用于优化单个业务的加边策略，但全局来看效果不好
    for (int i = 0; i < N; ++i) {
        minDist2start[i] = INF;
        vector<int>().swap(node[i].reachPile);
    }
    minDist2start[start] = 0;

    for (; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // 赋初值
            vis1[i] = false;
        }
        queue<pair<int, int>> null_queue;
        bfsQ.swap(null_queue);
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
                    tmpDist2start[t] = curLevel + 1;
                    if (tmpDist2start[t] < minDist2start[t]) {
                        minDist2start[t] = tmpDist2start[t];
                        node[t].reachPile.push_back(p);
                    }
                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        curLevel += 1;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }
                }
            }

        }
        if (s == end) {

            //if (p == 0) {
            //    choosenP = p;
            //    findPath = true;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    break;
            //}
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

        //findAddPath2(bus, dist2start); // 用于优化单个业务的加边策略，但全局来看效果不好
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

}

// 寻找业务bus的起点到终点的路径（不考虑通道堵塞），并对路径上的每一条边都执行加边操作，然后交给BFS1操作
void BFS2(Business& bus) {

    int start = bus.start, end = bus.end;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }

    queue<pair<int, int>> null_queue;
    bfsQ.swap(null_queue);
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
                curLevel += 1;
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

        curNode = edge[bus.trueMinPath[curNode]].from;
    }
    BFS1(bus);

}

// 寻找业务bus的起点到终点，在不考虑通道堵塞下的最短路径
void BFS3(Business& bus) {

    int start = bus.start, end = bus.end;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }

    queue<pair<int, int>> null_queue;
    bfsQ.swap(null_queue);
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
                curLevel += 1;
                break;
            }
            else {
                bfsQ.push(make_pair(t, curLevel + 1));
            }
        }

    }

}

// 寻找从start到end，在特定通道下的最短路径
void BFS4(int start, int end, int pileId, vector<int>& tmpOKPath) {

    if (start == end)
        return;
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }

    tmpOKPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }
    queue<pair<int, int>> null_queue;
    bfsQ.swap(null_queue);
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

            if (edge[i].Pile[pileId] == -1) {        // pile未被占用时，才试图走该边
                int t = edge[i].to;
                if (vis1[t])
                    continue;
                vis1[t] = true;
                tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                if (t == end) {
                    getOutFlag = true;
                    s = t;
                    curLevel += 1;
                    break;
                }
                else {
                    bfsQ.push(make_pair(t, curLevel + 1));
                }
            }
        }

    }

}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最短路径，因为有可能边的通道被完全占用）
void dijkstra1(Business& bus) {

    //dijkstra3(bus);
    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    // 用于优化单个业务的加边策略，但全局来看效果不好
    for (int i = 0; i < N; ++i) {
        vis2[i] = false;
        minDist2start[i] = INF;
        vector<int>().swap(node[i].reachPile);
    }
    vis2[start] = true;
    minDist2start[start] = 0;

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
            }

            // 没有遍历过才需要遍历
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // 搜索堆顶所有连边

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;

                    vis2[t] = true;
                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i
                        dis[t] = dis[s] + edge[i].d;   // 松弛操作
                        q.push(Node1(dis[t], t));   // 把新遍历到的点加入堆中

                        //vis2[t] = true;
                        if (dis[t] < minDist2start[t]) {
                            minDist2start[t] = dis[t];
                            node[t].reachPile.push_back(p);
                        }

                    }
                }
                else {

                }

            }
        }
        if (s == end) {

            //if (p == 0) {
            //    choosenP = p;
            //    findPath = true;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    break;
            //}
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

            ////当走p通道的路径是事实上的最短路径时，则结束p的循环
            //int curNode = end, tmpDist = 0;
            //bool breakFlag = true;
            //while (tmpOKPath[curNode] != -1) {

            //    if (bus.trueMinPath[curNode] != tmpOKPath[curNode]) {   // 与最短路径稍有不同，就不中断往下一个通道的搜索
            //        breakFlag = false;
            //    }
            //    int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
            //    curNode = edge[edgeId].from;
            //    tmpDist += edge[edgeId].d;
            //    if (curNode == start)
            //        break;
            //}
            //if (breakFlag) {
            //    choosenP = p;
            //    findPath = true;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    break;
            //}
            //if (tmpDist < minPathDist) {
            //    minPathDist = tmpDist;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    choosenP = p;
            //}
            //findPath = true;

        }
    }

    if (findPath == false) {    // 找不到路，需要构造新边，以下提供了两种策略，使用时需要注释掉其中一个

        findAddPath1(bus, vis2); // 用于优化单个业务的加边策略，但全局来看效果不好
        //dijkstra2(bus);       // 旧的加边策略，一但加边，整个路径都会加，但全局性能是当前最好的
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

    int curNode = end;
    while (bus.trueMinPath[curNode] != -1) {
        int edgeId = bus.trueMinPath[curNode];  // 存储于edge数组中真正的边的Id

        addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
        addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

        if (edge[edgeId].from < edge[edgeId].to)
            newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
        else
            newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);


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

void reverseArray(vector<int>& arr) {

    int tmp, n = arr.size();
    for (int i = 0; i < n / 2; ++i) {
        tmp = arr[n - i - 1];
        arr[n - i - 1] = arr[i];
        arr[i] = tmp;
    }

}

void outPut() {
    cout << newEdge.size() << endl;

    int totNewEdge = newEdge.size();
    int totPathSize = 0;
    int totMultiplier = 0;

    for (int i = 0; i < newEdge.size(); ++i) {
        cout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();

        totPathSize += pSize;
        totMultiplier += mSize;

        cout << buses[i].pileId << " " << pSize << " "
            << mSize << " ";
        for (int j = 0; j < pSize; ++j) {
            cout << buses[i].path[j];
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

    allNewEdgeCnt[testTime] = totNewEdge;
    allPathCnt[testTime] = totPathSize;
    allMultiplierCnt[testTime] = totMultiplier;
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
void findAddPath1(Business& bus, bool* vis2) {

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
                else {

                    int curMinDist = minDist2start[t];
                    OKPile = node[t].reachPile[node[t].reachPile.size() - 1];
                    if (!q.empty())
                        q.pop();

                    while (!q.empty()) {

                        t = q.front();      // t与
                        q.pop();
                        if (!vis2[t]) {

                            for (int j = head[t]; j != -1; j = edge[j].next) {
                                int newT = edge[j].to;
                                if (vis2[newT] && newT != bus.start) {
                                    if (minDist2start[newT] < curMinDist) {
                                        tmpOKPath[newT] = j;
                                        curMinDist = minDist2start[newT];
                                        firstOKPoint = newT;
                                        OKPile = node[newT].reachPile[node[newT].reachPile.size() - 1];
                                    }
                                }
                            }
                            continue;
                        }
                            
                        if (t == bus.start)
                            break;
                        
                        if (minDist2start[t] < curMinDist) {
                            curMinDist = minDist2start[t];
                            firstOKPoint = t;
                            OKPile = node[t].reachPile[node[t].reachPile.size() - 1];
                        }

                    }

                }

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

// 在因通道堵塞而添加不了业务时，寻找合适的位置进行加边操作，并再次寻路
void findAddPath2(Business& bus, int* dist2start) {

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

            if (dist2start[t] >= 0) {

                firstOKPoint = t;
                if (t == bus.start) {
                    OKPile = 0;
                    getOutFlag = true;
                    break;
                }
                else {

                    // 一旦搜索到有可行点，则不再进行q.push操作
                    // 而是在余下的顶点里，寻找距离起点最近的

                    OKPile = node[t].reachPile[node[t].reachPile.size() - 1];
                    getOutFlag = true;
                    break;
                }

            }
            else {  // 说明t点也是不可达点
                q.push(t);
            }

        }
    }

    BFS4(firstOKPoint, bus.start, OKPile, tmpOKPath);  // tmpOKPath存储的路径是从终点到起点的路径，使用时要注意

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
