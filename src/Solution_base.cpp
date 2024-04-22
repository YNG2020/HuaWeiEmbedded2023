#include "Solution.h"
#include "UtilityFunction.h"
#include "global_var.h"
#include "global_struct.h"
#include <iostream>
#include <cmath>
#include <queue>
#include <algorithm>
#include <vector>
#include <random>

// Solution初始化
Solution::Solution()
{
    tmpOKPath.resize(N);
}

// 光线扩容难题总策略
void Solution::runStrategy()
{
    preAllocateBus();

    if (Configure::forIterOutput && !Configure::forJudger)
        std::cout << "Original newEdge.size = " << newEdge.size() << endl;
    for (int cnt = 0; cnt < cntLimit; ++cnt)
    {
        reAllocateBus(pow(reAllocateBusNumFunBase, reAllocateBusNumFunExpRatio * cnt) * T);
        tryDeleteEdge();
        if (Configure::forIterOutput && !Configure::forJudger)
            std::cout << "newEdge.size = " << newEdge.size() << endl;
    }

    tryDeleteEdge();
    tryDeleteEdge();
    if (Configure::forIterOutput && !Configure::forJudger)
        std::cout << "newEdge.size = " << newEdge.size() << endl;
}

// 将所有的业务分配到光网络中
void Solution::preAllocateBus()
{
    for (int i = 0; i < T; ++i)
    {
        loadBus(i, true);
    }
}

// 试图重新分配业务到光网络中
void Solution::reAllocateBus(int HLim)
{
    int gap = max(int(0.025 * T), 20);       // (TODO，gap的机理需要被弄清楚)
    if (gap > T)
        return;
    vector<int> totBusIdx(T, 0);
    vector<int> busIdx(gap, 0);
    for (int i = 0; i < T; ++i)
        totBusIdx[i] = i;

    for (int i = 0; i + gap < HLim; i = i + gap)
    {
        std::mt19937 rng(42); // 设置随机数种子  
        random_shuffle(totBusIdx.begin(), totBusIdx.end());
        for (int i = 0; i < gap; ++i)
        {
            busIdx[i] = totBusIdx[i];
        }

        int oriEdgeNum = 0;

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busId; j < i + gap; ++j)
        {
            busId = busIdx[j - i];
            oriEdgeNum += buses[busId].path.size();
            pathTmp1[j - i] = buses[busId].pathTmp;
            pileTmp1[j - i] = buses[busId].pileId;
            recoverNetwork(busId, buses[busId].pileId);
        }

        int curEdgeNum = 0;
        bool findPath = false;
        vector<vector<int>> pathTmp2(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busId; j >= i; --j)
        {
            busId = busIdx[j - i];
            loadBus(busId, false);
            pathTmp2[j - i] = buses[busId].pathTmp;
            pileTmp2[j - i] = buses[busId].pileId;
            curEdgeNum += buses[busId].path.size();
        }

        if (oriEdgeNum > curEdgeNum)
        {   // 总体的边数减少，接受迁移
            continue;
        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, busId; j >= i; --j)
            {   // 把试图寻路时，造成的对网络的影响消除
                busId = busIdx[j - i];
                recoverNetwork(busId, pileTmp2[j - i]);
            }

            for (int j = i, busId; j < i + gap; ++j)
            {   // 重新加载所有的边
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
void Solution::tryDeleteEdge()
{
    int n = newEdge.size(), trueEdgeId;
    for (int idx = 0; idx < n; ++idx)
    {
        int idxEdge = newEdgePathId[idx]; // idxEdge为边在边集数组的编号（计数时，双向边视作同一边）  

        trueEdgeId = idxEdge * 2;
        int busCnt = 0;
        vector<int> lastBusIds, lastPileIds;

        for (int j = 0; j < P; ++j)
            if (edge[trueEdgeId].Pile[j] != -1 && edge[trueEdgeId].Pile[j] != T)
            {   // 说明在通道j上承载了该业务
                ++busCnt;
                lastBusIds.push_back(edge[trueEdgeId].Pile[j]);
                lastPileIds.push_back(j);
            }

        if (busCnt == 0) 
        {   // 如果该新边上，一条业务都没有承载，直接删边
            int iter = find(newEdgePathId.begin(), newEdgePathId.end(), idxEdge) - newEdgePathId.begin();
            newEdge.erase(newEdge.begin() + iter);
            newEdgePathId.erase(newEdgePathId.begin() + iter);
            --idx;
            --n;

            for (int k = 0; k < P; ++k)
            {   // 该边已删除，就应对其进行封锁
                edge[trueEdgeId].Pile[k] = T;
                edge[trueEdgeId + 1].Pile[k] = T;   // 偶数+1
            }
        }
        else
        {   // 如果该新边上，承载了多条业务，则对该边上的所有业务重新分配，看能否不依赖该新边
            vector<vector<int>> pathTmp(busCnt, vector<int>());     // 用于此后重新加载边
            for (int k = 0; k < busCnt; ++k)
            {
                pathTmp[k] = buses[lastBusIds[k]].pathTmp;
                recoverNetwork(lastBusIds[k], lastPileIds[k]);
            }

            bool findPath = false;
            int stopK = -1;
            vector<int> tmpLastPileIds;
            for (int k = 0; k < busCnt; ++k)
            {
                findPath = BFS_detectPath(buses[lastBusIds[k]], idxEdge);
                if (findPath == false)
                {
                    stopK = k;
                    break;
                }
                tmpLastPileIds.push_back(buses[lastBusIds[k]].pileId);   // 原本的pileId已改变，此处进行更新，以防止reCoverNetwork时出bug
            }

            if (findPath)
            {
                int iter = find(newEdgePathId.begin(), newEdgePathId.end(), idxEdge) - newEdgePathId.begin();
                newEdge.erase(newEdge.begin() + iter);
                newEdgePathId.erase(newEdgePathId.begin() + iter);
                --idx;
                --n;

                for (int k = 0; k < P; ++k)
                {   // 该边已删除，就应对其进行封锁
                    edge[trueEdgeId].Pile[k] = T;
                    edge[trueEdgeId + 1].Pile[k] = T;   // 偶数+1
                }

                for (int k = 0; k < busCnt; ++k)
                {
                    vector<int> nullVector;
                    buses[lastBusIds[k]].mutiplierId.swap(nullVector);
                    buses[lastBusIds[k]].curA = D;

                    int curNode = buses[lastBusIds[k]].start, trueNextEdgeId;
                    for (int i = 0; i < buses[lastBusIds[k]].path.size(); ++i)
                    {
                        if (edge[buses[lastBusIds[k]].path[i] * 2].from == curNode)
                            trueNextEdgeId = buses[lastBusIds[k]].path[i] * 2;
                        else
                            trueNextEdgeId = buses[lastBusIds[k]].path[i] * 2 + 1;
                        curNode = edge[trueNextEdgeId].to;

                        if (buses[lastBusIds[k]].curA >= edge[trueNextEdgeId].trueD)
                        {
                            buses[lastBusIds[k]].curA -= edge[trueNextEdgeId].trueD;
                        }
                        else
                        {
                            node[edge[trueNextEdgeId].from].Multiplier[buses[lastBusIds[k]].pileId] = buses[lastBusIds[k]].pileId;
                            buses[lastBusIds[k]].curA = D;
                            buses[lastBusIds[k]].curA -= edge[trueNextEdgeId].trueD;
                            buses[lastBusIds[k]].mutiplierId.push_back(edge[trueNextEdgeId].from);
                        }
                    }
                }

            }
            else
            {
                for (int k = 0; k < stopK; ++k)
                {   // 把试图寻路时，造成的对网络的影响消除
                    recoverNetwork(lastBusIds[k], tmpLastPileIds[k]);
                }

                for (int k = 0; k < busCnt; ++k)
                {   // 重新加载所有的边
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
void Solution::loadBus(int busId, bool ifTryDeleteEdge)
{
    BFS_loadBus(buses[busId], ifTryDeleteEdge);
    loadMultiplier(busId);
}

// 先清空原来的业务对网络的影响
void Solution::recoverNetwork(int lastBusID, int lastPileId)
{
    ///////////////////////////////////////////////////////////////////////
    // 清空对寻路的影响
    vector<int> pathTmp = buses[lastBusID].pathTmp;
    int curNode = buses[lastBusID].end;
    while (pathTmp[curNode] != -1)
    {
        int edgeId = pathTmp[curNode];  // 存储于edge数组中真正的边的Id
        edge[edgeId].Pile[lastPileId] = -1;
        --edge[edgeId].usedPileCnt;

        if (edgeId % 2)
        {   // 奇数-1
            edge[edgeId - 1].Pile[lastPileId] = -1;   // 双向边，两边一起处理
            --edge[edgeId - 1].usedPileCnt;
        }
        else
        {   // 偶数+1
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
void Solution::reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp)
{
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
    std::reverse(buses[lastBusID].path.begin(), buses[lastBusID].path.end());

    // 重新设置放大器
    loadMultiplier(lastBusID);
}

// 加载放大器到业务上
void Solution::loadMultiplier(int busId)
{
    int curNode = buses[busId].start, trueNextEdgeId;
    for (int i = 0; i < buses[busId].path.size(); ++i)
    {
        if (edge[buses[busId].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[busId].path[i] * 2;
        else
            trueNextEdgeId = buses[busId].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[busId].curA >= edge[trueNextEdgeId].trueD)
        {
            buses[busId].curA -= edge[trueNextEdgeId].trueD;
        }
        else
        {
            node[edge[trueNextEdgeId].from].Multiplier[buses[busId].pileId] = buses[busId].pileId;
            buses[busId].curA = D;
            buses[busId].curA -= edge[trueNextEdgeId].trueD;
            buses[busId].mutiplierId.push_back(edge[trueNextEdgeId].from);
        }
    }
}
