#include "Solution.h"
#include "UtilityFunction.h"
#include "global_var.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <vector>
#include <cstring>

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void Solution::BFS_loadBus(Business& bus, bool ifLoadNewEdge) {

    int start = bus.start, end = bus.end;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    static int addNewBus = 0;   // 加业务次数
    ++addNewBus;
    bool findPath = false;
    int minPathDist = Configure::INF;
    int choosenP = -1;
    double maxValue = -1.0;

    for (int p = 0; p < P; ++p)
    {   // P个编号的通道都搜索一次

        std::fill(tmpOKPath.begin(), tmpOKPath.end(), -1);      // 存储路径的数组初始化
        memset(vis, 0, sizeof(vis));                            // vis数组初始化
        queue<pair<int, int>> nodes;
        nodes.push(make_pair(start, 0));
        vis[start] = true;
        int from = start;
        int to = -1;
        int curDis = 0;

        while (!nodes.empty() && to != end) { // 队列为空即，所有点都被加入到生成树中去了

            from = nodes.front().first;
            curDis = nodes.front().second;
            nodes.pop();

            for (int i = head[from]; i != -1; i = edge[i].next) {

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    to = edge[i].to;
                    if (vis[to])
                        continue;
                    vis[to] = true;
                    tmpOKPath[to] = i;    // 记录下抵达路径点t的边的编号i

                    if (to == end) {
                        ++curDis;
                        break;
                    }
                    else {
                        nodes.push(make_pair(to, curDis + 1));
                    }

                }
            }

        }
        if (to == end) {

            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // 键不存在
                minPathSize[make_pair(start, end)] = curDis;
            else if (minPathSize[make_pair(start, end)] > curDis) {
                minPathSize[make_pair(start, end)] = curDis;
            }

            if (ifLoadNewEdge) {    // 如果BFS1在调用前已经添加了新边，则可以一遍过
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
                findPath = true;
                break;
            }

            if (curDis > 3 * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            int curNode = end, tmpDist = curDis;
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
    std::reverse(bus.path.begin(), bus.path.end());
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回
bool Solution::BFS_detectPath(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end;

    bool findPath = false;
    int minPathDist = Configure::INF;
    int choosenP = -1;

    for (int p = 0; p < P; ++p) {

        std::fill(tmpOKPath.begin(), tmpOKPath.end(), -1);      // 存储路径的数组初始化
        memset(vis, 0, sizeof(vis));                            // vis数组初始化

        queue<pair<int, int>> nodes;
        nodes.push(make_pair(start, 0));
        vis[start] = true;
        int s = start;
        int curLevel = 0;
        bool getOutFlag = false;

        while (!nodes.empty() && !getOutFlag) { // 队列为空即，所有点都被加入到生成树中去了

            s = nodes.front().first;
            curLevel = nodes.front().second;
            nodes.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1) {        // pile未被占用时，才试图走该边
                    int t = edge[i].to;
                    if (vis[t])
                        continue;
                    vis[t] = true;
                    tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        ++curLevel;
                        break;
                    }
                    else {
                        nodes.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (s == end)
        {
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
    std::reverse(bus.path.begin(), bus.path.end());
    return true;
}

// 不能加载业务时，寻找业务bus的起点到终点的路径（不考虑通道堵塞，全通道搜索），并对路径上的发生堵塞的边进行加边处理
void Solution::BFS_addNewEdge(Business& bus) {

    int start = bus.start, end = bus.end;
    int minBlockEdge = Configure::INF;
    int choosenP = -1;
    std::fill(tmpOKPath.begin(), tmpOKPath.end(), -1);      // 存储路径的数组初始化
    memset(vis, 0, sizeof(vis));                            // vis数组初始化
    queue<pair<int, int>> nodes;
    nodes.push(make_pair(start, 0));
    vis[start] = true;
    int s = start;
    int curLevel = 0;
    bool getOutFlag = false;

    while (!nodes.empty() && !getOutFlag) {

        s = nodes.front().first;
        curLevel = nodes.front().second;
        nodes.pop();

        for (int i = head[s]; i != -1; i = edge[i].next) {

            int t = edge[i].to;
            if (vis[t])
                continue;
            vis[t] = true;
            tmpOKPath[t] = i;    // 记录下抵达路径点t的边的编号i

            if (t == end) {
                getOutFlag = true;
                s = t;
                ++curLevel;
                break;
            }
            else
                nodes.push(make_pair(t, curLevel + 1));
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
    std::reverse(bus.path.begin(), bus.path.end());
}
