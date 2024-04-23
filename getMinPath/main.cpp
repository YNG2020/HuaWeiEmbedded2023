#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <math.h>
#include <time.h>
#include <cmath>
#include <fstream>
const int INF = 2147483647;

using namespace std;

int N, M, T, P, D; // 节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D
const int maxM = 50000; // 边的最大数目
const int maxN = 5000;  // 节点的最大数目
const int maxBus = 10000;   // 业务的最大数目
const int maxP = 80;    // 最大单边通道数量P
int cntEdge = 0;    // 当前边集数组所存储的边的数目
int cntBus = 0;     // 当前业务数组所存储业务的数目

class Edge {
public:
    int from, to, d, next;    // 起点，终点，边的距离，同起点的下一条边在edge中的编号
    int trueD;    // 边的真正距离
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
    Business() {
        start = -1;
        end = -1;
    }
    vector<int> pathTmp;   // 存储从起点到其它点的最短路径的末边的编号（考虑通道堵塞的最短）
    vector<int> trueMinPath;   // 存储从起点到其它点的最短路径的末边的编号（不考虑通道堵塞的最短）
    vector<int> path;   // 存储路径所经过的边
}buses[maxBus];

int head[maxN]; // head[i]，表示以i为起点的在逻辑上的第一条边在边集数组的位置（编号）
bool vis1[maxN];  // 标识该点有无被访问过

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void BFS1(Business& bus, bool ifLoadNewEdge);
void loadBus(int busId, bool ifLoadRemain);
void allocateBus();
void reverseArray(vector<int>& arr);
void outPut();

// 主函数
int main() {

    ifstream cin("dataMATLAB.txt");
    cin >> N >> M >> T >> P >> D;
    init();
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        cin >> s >> t >> d;
        addEdge(s, t, d);
        addEdge(t, s, d);   // 添加双向边
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        cin >> Sj >> Tj;
        addBus(Sj, Tj); // 添加业务
    }

    allocateBus();
    outPut();

    return 0;
}

// 初始化
void init() {
    for (int i = 0; i < N; ++i)
        head[i] = -1;
}

// 将所有的业务分配到光网络中
void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i, false);
    }
}

// 把业务busId加载到光网络中
void loadBus(int busId, bool ifLoadRemain) {
    BFS1(buses[busId], false);
}

// 加边函数，s起点，t终点，d距离
void addEdge(int s, int t, int d) {
    edge[cntEdge].from = s; // 起点
    edge[cntEdge].to = t;   // 终点
    edge[cntEdge].d = 1;    // 距离
    edge[cntEdge].trueD = d;    // 距离
    edge[cntEdge].next = head[s];   // 链式前向。以s为起点下一条边的编号，head[s]代表的是当前以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    head[s] = cntEdge++;    // 更新以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
}

// 加业务函数
void addBus(int start, int end) {
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    vector<int>().swap(buses[cntBus].path);
    vector<int>().swap(buses[cntBus].pathTmp);
    ++cntBus;
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void BFS1(Business& bus, bool ifLoadNewEdge) {

    int start = bus.start, end = bus.end;
    vector<int> tmpOKPath;

    tmpOKPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // 赋初值
        vis1[i] = false;
    }
    queue<int> bfsQ;
    bfsQ.push(start);
    vis1[start] = true;
    int s = start;
    int t = -1;

    while (!bfsQ.empty() && t != end) { // 队列为空即，所有点都被加入到生成树中去了

        s = bfsQ.front();
        bfsQ.pop();

        for (int i = head[s]; i != -1; i = edge[i].next) {

            t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

            if (t == end)
                break;
            else
                bfsQ.push(t);
        }

    }
    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());

    int curNode = end;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        curNode = edge[edgeId].from;
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

    ofstream cout("businessAllocation.txt");
    if (!cout.is_open()) {
        cout << "Error opening file." << endl;
    }
    cout << T << endl;
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();

        cout << pSize << " ";
        for (int j = 0; j < pSize; ++j) {
            int pathID = buses[i].path[j];
            cout << pathID;
            if (j == pSize - 1 && i != T - 1) {
                cout << endl;
            }
            else
                cout << " ";
        }

    }
}
