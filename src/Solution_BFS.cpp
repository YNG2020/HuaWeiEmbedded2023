#include "Solution.h"
#include "UtilityFunction.h"
#include "global_var.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <vector>
#include <cstring>

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void Solution::BFS_loadBus(Business& bus, bool ifTryDeleteEdge)
{
    int start = bus.start, end = bus.end;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    static int addNewBus = 0;      // 加业务次数
    ++addNewBus;
    bool findPath = false;
    int minPathDist = Configure::INF;
    int choosenP = -1;

    // P个编号的通道都搜索一次
    for (int p = 0; p < P; ++p)
    {   
        // 相关寻路变量的初始化
        std::fill(tmpOKPath.begin(), tmpOKPath.end(), -1);      // 存储路径的数组初始化
        memset(vis, 0, sizeof(vis));                            // vis数组初始化
        queue<SimpleNode> nodes;
        nodes.emplace(start, 0);
        vis[start] = true;
        int from = start, to = -1, curDis = 0;

        while (!nodes.empty() && to != end)
        {   // 队列为空即，所有点都被加入到生成树中去了
            from = nodes.front().nodeID;
            curDis = nodes.front().dis;
            nodes.pop();

            for (int i = head[from]; i != -1; i = edge[i].next)
            {
                if (edge[i].Pile[p] == -1)
                {   // pile未被占用时，才试图走该边
                    to = edge[i].to;
                    if (vis[to])
                        continue;
                    vis[to] = true;
                    tmpOKPath[to] = i;    // 记录下抵达路径点t的边的编号i

                    if (to == end)
                    {
                        ++curDis;
                        break;
                    }
                    else
                    {
                        nodes.emplace(to, curDis + 1);
                    }
                }
            }
        }
        if (to == end)
        {   // 以下对找到的业务路径进行路径效率判断
            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // 键不存在 (TODO，背后的原理需要摸清，以判断参数在不同情况下的表现)
                minPathSize[make_pair(start, end)] = curDis;
            else if (minPathSize[make_pair(start, end)] > curDis)
            {
                minPathSize[make_pair(start, end)] = curDis;
            }

            if (curDis > pathSizeLimRatio * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            int curNode = end, tmpDist = curDis;
            if (tmpDist < minPathDist)
            {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false)
    {   // 找不到路，需要构造新边
        if (ifTryDeleteEdge)
        {
            if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))   // 删边时机控制，不过度删边，不在添加相同的业务时删边
                tryDeleteEdge();
        }
        BFS_addNewEdge(bus);       // 只加最短路径上需要进行加边的边
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1)
    {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id

        bus.path.push_back(edgeId / 2);     // edgeId / 2是为了适应题目要求
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2)
        {   // 奇数-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
            ++edge[edgeId - 1].usedPileCnt;
        }
        else
        {   // 偶数+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[edgeId].from;
    }
    std::reverse(bus.path.begin(), bus.path.end());
}

// 考虑一边多通道的情况下，寻找业务bus的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回
bool Solution::BFS_detectPath(Business& bus, int blockEdge)
{
    int start = bus.start, end = bus.end;

    bool findPath = false;
    int minPathDist = Configure::INF;
    int choosenP = -1;

    for (int p = 0; p < P; ++p)
    {
        // 相关寻路变量的初始化
        std::fill(tmpOKPath.begin(), tmpOKPath.end(), -1);      // 存储路径的数组初始化
        memset(vis, 0, sizeof(vis));                            // vis数组初始化
        queue<SimpleNode> nodes;
        nodes.emplace(start, 0);
        vis[start] = true;
        int from = start, to = -1, curDis = 0;

        while (!nodes.empty() && to != end)
        {   // 队列为空即，所有点都被加入到生成树中去了
            from = nodes.front().nodeID;
            curDis = nodes.front().dis;
            nodes.pop();

            for (int i = head[from]; i != -1; i = edge[i].next)
            {
                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1)
                {   // pile未被占用时，才试图走该边
                    to = edge[i].to;
                    if (vis[to])
                        continue;
                    vis[to] = true;
                    tmpOKPath[to] = i;    // 记录下抵达路径点to的边的编号i

                    if (to == end)
                    {
                        from = to;
                        ++curDis;
                        break;
                    }
                    else
                    {
                        nodes.emplace(to, curDis + 1);
                    }

                }
            }
        }
        if (from == end)
        {
            // 找到的路径长度太长，宁愿不要
            if (curDis > pathSizeLimRatio * minPathSize[make_pair(start, end)])
                continue;

            int curNode = end, tmpDist = curDis;
            if (tmpDist <= minPathDist)
            {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false)
    {   // 找不到路，需要构造新边
        return false;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1)
    {
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
void Solution::BFS_addNewEdge(Business& bus)
{
    // 相关寻路变量的初始化
    int start = bus.start, end = bus.end;
    int minBlockEdgeCnt = Configure::INF;                   // 记录不同通道编号下，对于某一个光业务的最短路径，该路径上已经被占用的边的最小数量
    int choosenP = -1;
    std::fill(tmpOKPath.begin(), tmpOKPath.end(), -1);      // 存储路径的数组初始化
    memset(vis, 0, sizeof(vis));                            // vis数组初始化
    queue<SimpleNode> nodes;
    nodes.emplace(start, 0);
    vis[start] = true;
    int from = start, to = -1, curDis = 0;

    while (!nodes.empty() && to != end)
    {
        from = nodes.front().nodeID;
        curDis = nodes.front().dis;
        nodes.pop();

        for (int i = head[from]; i != -1; i = edge[i].next)
        {
            to = edge[i].to;
            if (vis[to])
                continue;
            vis[to] = true;
            tmpOKPath[to] = i;    // 记录下抵达路径点to的边的编号i

            if (to == end)
            {
                from = to;
                ++curDis;
                break;
            }
            else
                nodes.emplace(to, curDis + 1);
        }

    }
    for (int p = 0; p < P; ++p)
    {
        int curNode = end, tmpBlockEdgeCnt = 0;
        while (tmpOKPath[curNode] != -1)
        {
            int edgeId = tmpOKPath[curNode];  // 存储于edge数组中真正的边的Id
            if (edge[edgeId].Pile[p] != -1)
                ++tmpBlockEdgeCnt;
            curNode = edge[edgeId].from;
        }

        if (tmpBlockEdgeCnt < minBlockEdgeCnt)
        {   // 选需要加边数最少的通道
            minBlockEdgeCnt = tmpBlockEdgeCnt;
            bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            choosenP = p;
        }
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1)
    {
        int edgeId = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        int lastNode = curNode;
        curNode = edge[bus.pathTmp[curNode]].from;

        if (edge[edgeId].Pile[choosenP] == -1)
        {    // 无需加边
            bus.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
            edge[edgeId].Pile[choosenP] = bus.busId;
            ++edge[edgeId].usedPileCnt;

            if (edgeId % 2)
            {   // 奇数-1
                edge[edgeId - 1].Pile[choosenP] = bus.busId;   // 双向边，两边一起处理
                ++edge[edgeId - 1].usedPileCnt;
            }
            else
            {  // 偶数+1
                edge[edgeId + 1].Pile[choosenP] = bus.busId;
                ++edge[edgeId + 1].usedPileCnt;
            }
        }
        else 
        {      // 需要加边
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
