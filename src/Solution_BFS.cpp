#include "Solution.h"
#include "UtilityFunction.h"
#include "global_var.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <vector>
#include <cstring>

// 考虑一边多通道的情况下，寻找业务tran的起点到终点的路径（不一定是最少边数路径，因为有可能边的通道被完全占用）
void Solution::BFS_loadTran(Transaction& tran, bool ifTryDeleteEdge)
{
    int start = tran.start, end = tran.end;
    static int addNewEdgeCnt = 0;  // 加边次数（不是边数）
    static int addNewTran = 0;      // 加业务次数
    ++addNewTran;
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

            for (int i = tail[from]; i != -1; i = edge[i].pre)
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
            if (forNoDetour && curDis > pathSizeLimRatio * minPathSize[make_pair(start, end)])  // 找到的路径长度太长，宁愿不要
                continue;

            int curNode = end, tmpDist = curDis;
            if (tmpDist < minPathDist)
            {
                minPathDist = tmpDist;
                tran.lastEdgesOfShortestPaths = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false)
    {   // 找不到路，需要构造新边
        if (ifTryDeleteEdge)
        {
            if (tran.start != trans[tran.tranID - 1].start || tran.end != trans[tran.tranID - 1].end)   // 删边时机控制，不过度删边，不在添加相同的业务时删边
                tryDeleteEdge();
        }
        BFS_addNewEdge(tran);       // 只加最短路径上需要进行加边的边
        return;
    }

    tran.pileID = choosenP;
    backtrackPath(tran);     // 回溯路径，以构造出一条完整的路径
    transferTranInMultiEdge(tran);
}

// 考虑一边多通道的情况下，寻找业务tran的起点到终点的路径，但遇到需要加边的情况，不做处理，直接返回，仅tryDeletegeEdge时使用
bool Solution::BFS_detectPath(Transaction& tran, int blockEdge)
{
    int start = tran.start, end = tran.end;

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

            for (int i = tail[from]; i != -1; i = edge[i].pre)
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
            int curNode = end, tmpDist = curDis;
            if (tmpDist <= minPathDist)
            {
                minPathDist = tmpDist;
                tran.lastEdgesOfShortestPaths = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false)
    {   // 找不到路，直接返回
        return false;
    }

    tran.pileID = choosenP;
    backtrackPath(tran);     // 回溯路径，以构造出一条完整的路径
    return true;
}

// 不能加载业务时，寻找业务tran的起点到终点的路径（不考虑通道堵塞，全通道搜索），并对路径上的发生堵塞的边进行加边处理
void Solution::BFS_addNewEdge(Transaction& tran)
{
    // 相关寻路变量的初始化
    int start = tran.start, end = tran.end;
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

        for (int i = tail[from]; i != -1; i = edge[i].pre)
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
    vector<vector<pair<int, int>>> nodeEdgePair(P);
    for (int p = 0; p < P; ++p)
    {
        int curNode = end, tmpBlockEdgeCnt = 0;
        while (tmpOKPath[curNode] != -1)
        {
            int edgeID = tmpOKPath[curNode];  // 存储于edge数组中真正的边的ID
            if (edge[edgeID].Pile[p] != -1)
            {
    //            int from = edge[edgeID].from, to = edge[edgeID].to;
    //            pair<int, int> nodePair = make_pair(from, to);
    //            int multiEdgeSize = multiEdgeID[nodePair].size();
    //            int i = 0;
    //            for (; i < multiEdgeSize; ++i)
    //            {
				//	int multiEdgeId = multiEdgeID[nodePair][i];
    //                if (edge[multiEdgeId].Pile[p] == -1)
    //                {   // 这条重边在p通道上没有被占用，则把它更新为最短路径上的边
    //                    if (edge[multiEdgeId].from == from && edge[multiEdgeId].to == to)
    //                    {
    //                        tmpOKPath[curNode] = multiEdgeId;
    //                        nodeEdgePair[p].emplace_back(curNode, multiEdgeId);
    //                    }
    //                    else
    //                    {
    //                        tmpOKPath[curNode] = multiEdgeId + 1;
    //                        nodeEdgePair[p].emplace_back(curNode, multiEdgeId + 1);
    //                    }
    //                    break;
    //                }
				//}
                //if (i == multiEdgeSize)
                    ++tmpBlockEdgeCnt;
            }
                
            curNode = edge[edgeID].from;
        }

        if (tmpBlockEdgeCnt < minBlockEdgeCnt)
        {   // 选需要加边数最少的通道
            minBlockEdgeCnt = tmpBlockEdgeCnt;
            choosenP = p;
        }
    }

    for (int i = 0; i < nodeEdgePair[choosenP].size(); ++i)
    {   // 更新最短路径上的边，更新为最佳重边（无须在该重边上执行加边操作）
        int nodeID = nodeEdgePair[choosenP][i].first;
        int edgeID = nodeEdgePair[choosenP][i].second;
        tmpOKPath[nodeID] = edgeID;
    }

    tran.lastEdgesOfShortestPaths = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
    int curNode = end;
    tran.pileID = choosenP;
    while (tran.lastEdgesOfShortestPaths[curNode] != -1)
    {
        int edgeID = tran.lastEdgesOfShortestPaths[curNode];  // 存储于edge数组中真正的边的ID
        int lastNode = curNode;
        curNode = edge[tran.lastEdgesOfShortestPaths[curNode]].from;

        if (edge[edgeID].Pile[choosenP] == -1)
        {    // 无需加边
            tran.path.push_back(edgeID / 2); // edgeID / 2是为了适应题目要求
            edge[edgeID].Pile[choosenP] = tran.tranID;
            ++edge[edgeID].usedPileCnt;

            if (edgeID % 2)
            {   // 奇数-1
                edge[edgeID - 1].Pile[choosenP] = tran.tranID;   // 双向边，两边一起处理
                ++edge[edgeID - 1].usedPileCnt;
            }
            else
            {  // 偶数+1
                edge[edgeID + 1].Pile[choosenP] = tran.tranID;
                ++edge[edgeID + 1].usedPileCnt;
            }
        }
        else 
        {      // 需要加边
            addEdge(edge[edgeID].from, edge[edgeID].to, minDist[make_pair(edge[edgeID].from, edge[edgeID].to)]);
            addEdge(edge[edgeID].to, edge[edgeID].from, minDist[make_pair(edge[edgeID].to, edge[edgeID].from)]);

            if (edge[edgeID].from < edge[edgeID].to)
                newEdge.emplace_back(edge[edgeID].from, edge[edgeID].to);
            else
                newEdge.emplace_back(edge[edgeID].to, edge[edgeID].from);
            newEdgePathID.emplace_back(cntEdge / 2 - 1);

            tran.path.push_back(cntEdge / 2 - 1); // edgeID / 2是为了适应题目要求
            edge[cntEdge - 2].Pile[choosenP] = tran.tranID;
            ++edge[cntEdge - 2].usedPileCnt;
            edge[cntEdge - 1].Pile[choosenP] = tran.tranID;   // 偶数+1
            ++edge[cntEdge - 1].usedPileCnt;
            tran.lastEdgesOfShortestPaths[lastNode] = cntEdge - 2;
        }

    }
    transferTranInMultiEdge(tran);
    std::reverse(tran.path.begin(), tran.path.end());
}

// 在不考虑通道堵塞的情况下，对业务Tran进行路径分配，以统计每个业务的路径长度
void Solution::BFS_tranStatistic(Transaction& tran)
{
    int start = tran.start, end = tran.end;
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

        for (int i = tail[from]; i != -1; i = edge[i].pre)
        {
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
    tran.lastEdgesOfShortestPaths = vector<int>(tmpOKPath.begin(), tmpOKPath.end());

    int curNode = end;
    while (tran.lastEdgesOfShortestPaths[curNode] != -1)
    {
        int edgeId = tran.lastEdgesOfShortestPaths[curNode];  // 存储于edge数组中真正的边的Id
        tran.path.push_back(edgeId / 2); // edgeId / 2是为了适应题目要求
        curNode = edge[edgeId].from;
    }
    std::reverse(tran.path.begin(), tran.path.end());
    for (int i = 0; i < tran.path.size(); ++i)
    {
        int edgeId = tran.path[i];
        edgeId = edgeId * 2;
		++edge[edgeId].usedPileCnt;
		++edge[edgeId + 1].usedPileCnt;
    }
    minPathSize[make_pair(start, end)] = tran.path.size();
}
