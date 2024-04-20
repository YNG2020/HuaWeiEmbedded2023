#pragma once
#include <vector>
#include "Configure.h"
#include <unordered_map>

using namespace std;
class Node {
public:
    int NodeId;           // 实际上利用数组的下标就可以唯一标识Node，这里先做一个冗余
    int Multiplier[Configure::maxP]; // 该节点上存在的放大器，记录的是当前要放大的通道的编号，放大器不存在时值为-1
    vector<int> reachPile;  // 在单次dijkstra搜索中，可抵达该顶点的pile编号
};

class Edge {
public:
    int from;   // 起点
    int to;     // 终点
    int d;      // 边的长度（置1时，可被用于dijkstra，此时相当于BFS）
    int next;   // 同起点的下一条边在edge中的编号
    int trueD;  // 边的真正长度，用于计算边的损耗，以添加放大器
    int Pile[Configure::maxP]; // 该边上存在的通道，记录的是当前承载的业务的编号，不承载业务时值为-1
    int usedPileCnt;
    Edge() {
        from = -1;
        to = -1;
        d = 0;
        next = -1;
    }

};    // 边集数组

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
};

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

extern Node node[Configure::maxN]; // 使用Configure.h中定义的maxN
extern Edge edge[Configure::maxM]; // 使用Configure.h中定义的maxM
extern Business buses[Configure::maxBus]; // 使用Configure.h中定义的maxBus

extern unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;   // 记录两个节点间的最短边的长度
extern unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;   // 记录两个节点间的最短路径长度