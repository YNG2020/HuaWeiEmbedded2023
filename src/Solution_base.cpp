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
    // 首先获取业务路径（不考虑通道堵塞）在光网络上的分布情况
    runStatistic();
    if (Configure::forStatisticOutput && !Configure::forJudger)
    {   
        outputStatistic();
    }
    sortBus();  // 根据 expectedAllocationPressure 对加载业务的顺序进行排序
    preAllocateBus();
    tryDeleteEdge();

    if (forIter)
    {
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
    }

    if (Configure::forIterOutput && !Configure::forJudger)
        std::cout << "newEdge.size = " << newEdge.size() << endl;
}

// 对业务路径在网络上的分布做一个统计（不考虑通道编号限制）
void Solution::runStatistic()
{
    for (int i = 0; i < T; ++i)
		BFS_busStatistic(buses[i]);
    for (int i = 0; i < T; ++i)
    {
        Business& bus = buses[i];
        int pathSize = bus.pathStatistic.size();
        for (int j = 0; j < pathSize; ++j)
        {
            bus.expectedAllocationPressure += (edge[bus.pathStatistic[j] * 2].statisticCnt % P);
        }
        //bus.expectedAllocationPressure = bus.pathStatistic.size();
    }
}

// 将所有的业务分配到光网络中
void Solution::preAllocateBus()
{
    for (int i = 0; i < T; ++i)
    {
        loadBus(sortedBusIndices[i], true);
    }
}

// 试图重新分配业务到光网络中
void Solution::reAllocateBus(int HLim)
{
    int gap = max(int(0.025 * T), 20);       // (TODO，gap的机理需要被弄清楚)
    if (gap > T)
        return;
    vector<int> totBusIDx(T, 0);
    vector<int> busIDx(gap, 0);
    for (int i = 0; i < T; ++i)
        totBusIDx[i] = i;

    for (int i = 0; i + gap < HLim; i = i + gap)
    {
        std::mt19937 rng(42); // 设置随机数种子  
        random_shuffle(totBusIDx.begin(), totBusIDx.end());
        for (int i = 0; i < gap; ++i)
        {
            busIDx[i] = totBusIDx[i];
        }

        int oriEdgeNum = 0;

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busID; j < i + gap; ++j)
        {
            busID = busIDx[j - i];
            oriEdgeNum += buses[busID].path.size();
            pathTmp1[j - i] = buses[busID].pathTmp;
            pileTmp1[j - i] = buses[busID].pileID;
            recoverNetwork(busID, buses[busID].pileID);
        }

        int curEdgeNum = 0;
        bool findPath = false;
        vector<vector<int>> pathTmp2(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busID; j >= i; --j)
        {
            busID = busIDx[j - i];
            loadBus(busID, false);
            pathTmp2[j - i] = buses[busID].pathTmp;
            pileTmp2[j - i] = buses[busID].pileID;
            curEdgeNum += buses[busID].path.size();
        }

        if (oriEdgeNum > curEdgeNum)
        {   // 总体的边数减少，接受迁移
            continue;
        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, busID; j >= i; --j)
            {   // 把试图寻路时，造成的对网络的影响消除
                busID = busIDx[j - i];
                recoverNetwork(busID, pileTmp2[j - i]);
            }

            for (int j = i, busID; j < i + gap; ++j)
            {   // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                busID = busIDx[j - i];
                buses[busID].mutiplierID.swap(nullVector);
                buses[busID].path.swap(nullPath1);
                buses[busID].pathTmp.swap(nullPath2);

                buses[busID].pileID = -1;
                buses[busID].curA = D;
                reloadBus(busID, pileTmp1[j - i], pathTmp1[j - i]);
            }
        }
    }
}

// 试图删除新边
void Solution::tryDeleteEdge()
{
    if (!forTryDeleteEdge)
		return;
    int n = newEdge.size(), trueEdgeID;
    for (int idx = 0; idx < n; ++idx)
    {
        int idxEdge = newEdgePathID[idx]; // idxEdge为边在边集数组的编号（计数时，双向边视作同一边）  

        trueEdgeID = idxEdge * 2;
        int busCnt = 0;
        vector<int> lastBusIDs, lastPileIDs;        // 存储将要被删除的边上承载的业务ID和通道ID

        for (int j = 0; j < P; ++j)
            if (edge[trueEdgeID].Pile[j] != -1 && edge[trueEdgeID].Pile[j] != T)
            {   // 说明在通道j上承载了该业务
                ++busCnt;
                lastBusIDs.push_back(edge[trueEdgeID].Pile[j]);
                lastPileIDs.push_back(j);
            }

        if (busCnt == 0) 
        {   // 如果该新边上，一条业务都没有承载，直接删边
            int iter = find(newEdgePathID.begin(), newEdgePathID.end(), idxEdge) - newEdgePathID.begin();
            newEdge.erase(newEdge.begin() + iter);
            newEdgePathID.erase(newEdgePathID.begin() + iter);
            --idx;
            --n;

            for (int k = 0; k < P; ++k)
            {   // 该边已删除，就应对其进行封锁
                edge[trueEdgeID].Pile[k] = T;
                edge[trueEdgeID + 1].Pile[k] = T;   // 偶数+1
            }
        }
        else
        {   // 如果该新边上，承载了多条业务，则对该边上的所有业务重新分配，看能否不依赖该新边
            vector<vector<int>> pathTmp(busCnt, vector<int>());     // 用于此后重新加载边
            for (int k = 0; k < busCnt; ++k)
            {
                pathTmp[k] = buses[lastBusIDs[k]].pathTmp;
                recoverNetwork(lastBusIDs[k], lastPileIDs[k]);
            }

            bool findPath = false;
            int stopK = -1;
            vector<int> tmpLastPileIDs;                                 // 作用见该变量被使用时对应的注释
            for (int k = 0; k < busCnt; ++k)
            {
                findPath = BFS_detectPath(buses[lastBusIDs[k]], idxEdge);
                if (findPath == false)
                {
                    stopK = k;
                    break;
                }
                tmpLastPileIDs.push_back(buses[lastBusIDs[k]].pileID);   // 原本的pileID已改变，此处进行更新，以防止reCoverNetwork时出bug
            }

            if (findPath)
            {   // 成功删除新边
                // 将原本新添加的边从newEdge中删除
                int iter = find(newEdgePathID.begin(), newEdgePathID.end(), idxEdge) - newEdgePathID.begin();
                newEdge.erase(newEdge.begin() + iter);
                newEdgePathID.erase(newEdgePathID.begin() + iter);
                --idx;
                --n;

                for (int k = 0; k < P; ++k)
                {   // 该边已删除，就应对其进行封锁
                    edge[trueEdgeID].Pile[k] = T;
                    edge[trueEdgeID + 1].Pile[k] = T;   // 偶数+1
                }

                for (int k = 0; k < busCnt; ++k)
                {   // 重新加载被迁移的业务的路径上的放大器设置
                    vector<int> nullVector;
                    buses[lastBusIDs[k]].mutiplierID.swap(nullVector);
                    buses[lastBusIDs[k]].curA = D;
                    loadMultiplier(lastBusIDs[k]);
                }
            }
            else
            {
                for (int k = 0; k < stopK; ++k)
                {   // 把试图寻路时，造成的对网络的影响消除
                    recoverNetwork(lastBusIDs[k], tmpLastPileIDs[k]);
                }

                for (int k = 0; k < busCnt; ++k)
                {   // 重新加载所有的边
                    vector<int> nullVector, nullPath1, nullPath2;
                    buses[lastBusIDs[k]].mutiplierID.swap(nullVector);
                    buses[lastBusIDs[k]].path.swap(nullPath1);
                    buses[lastBusIDs[k]].pathTmp.swap(nullPath2);

                    buses[lastBusIDs[k]].pileID = -1;
                    buses[lastBusIDs[k]].curA = D;
                    reloadBus(lastBusIDs[k], lastPileIDs[k], pathTmp[k]);
                }
            }
        }
    }
}

// 把业务busID加载到光网络中
void Solution::loadBus(int busID, bool ifTryDeleteEdge)
{
    BFS_detectMinPathSize(buses[busID]);        // 先找到最短路径的长度，以便于后续的路径长度限制
    BFS_loadBus(buses[busID], ifTryDeleteEdge); // 加载业务到光网络中
    loadMultiplier(busID);                      // 加载放大器到业务上
}

// 清空原来的业务对网络的影响
void Solution::recoverNetwork(int busID, int pileID)
{
    ///////////////////////////////////////////////////////////////////////
    // 清空对寻路的影响
    vector<int> pathTmp = buses[busID].pathTmp;
    int curNode = buses[busID].end;
    while (pathTmp[curNode] != -1)
    {
        int edgeID = pathTmp[curNode];  // 存储于edge数组中真正的边的ID
        edge[edgeID].Pile[pileID] = -1;
        --edge[edgeID].usedPileCnt;

        if (edgeID % 2)
        {   // 奇数-1
            edge[edgeID - 1].Pile[pileID] = -1;   // 双向边，两边一起处理
            --edge[edgeID - 1].usedPileCnt;
        }
        else
        {   // 偶数+1
            edge[edgeID + 1].Pile[pileID] = -1;
            --edge[edgeID + 1].usedPileCnt;
        }

        curNode = edge[pathTmp[curNode]].from;
    }

    //////////////////////////////////////////////////////////////////////
    vector<int> nullVector, nullPath1, nullPath2;
    buses[busID].mutiplierID.swap(nullVector);
    buses[busID].path.swap(nullPath1);
    buses[busID].pathTmp.swap(nullPath2);

    buses[busID].pileID = -1;
    buses[busID].curA = D;
}

// 重新将某单条业务加载到光网络上
void Solution::reloadBus(int busID, int pileID, vector<int>& pathTmp)
{
    // 重新设置路径
    buses[busID].pathTmp = vector<int>(pathTmp.begin(), pathTmp.end());

    int curNode = buses[busID].end;
    buses[busID].pileID = pileID;
    while (buses[busID].pathTmp[curNode] != -1) {
        int edgeID = buses[busID].pathTmp[curNode];     // 存储于edge数组中真正的边的ID

        buses[busID].path.push_back(edgeID / 2);        // edgeID / 2是为了适应题目要求
        edge[edgeID].Pile[pileID] = buses[busID].busID;
        ++edge[edgeID].usedPileCnt;

        if (edgeID % 2) {   // 奇数-1
            edge[edgeID - 1].Pile[pileID] = buses[busID].busID;   // 双向边，两边一起处理
            ++edge[edgeID - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeID + 1].Pile[pileID] = buses[busID].busID;
            ++edge[edgeID + 1].usedPileCnt;
        }

        curNode = edge[edgeID].from;
    }
    std::reverse(buses[busID].path.begin(), buses[busID].path.end());

    // 重新设置放大器
    loadMultiplier(busID);
}

// 加载放大器到业务上
void Solution::loadMultiplier(int busID)
{
    int curNode = buses[busID].start, trueNextEdgeID;
    for (int i = 0; i < buses[busID].path.size(); ++i)
    {
        if (edge[buses[busID].path[i] * 2].from == curNode)
            trueNextEdgeID = buses[busID].path[i] * 2;
        else
            trueNextEdgeID = buses[busID].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeID].to;

        if (buses[busID].curA >= edge[trueNextEdgeID].trueD)
        {
            buses[busID].curA -= edge[trueNextEdgeID].trueD;
        }
        else
        {
            node[edge[trueNextEdgeID].from].Multiplier[buses[busID].pileID] = buses[busID].pileID;
            buses[busID].curA = D;
            buses[busID].curA -= edge[trueNextEdgeID].trueD;
            buses[busID].mutiplierID.push_back(edge[trueNextEdgeID].from);
        }
    }
}

// 回溯路径，以构造出一条完整的路径
void Solution::backtrackPath(Business& bus)
{
    int curNode = bus.end;
    int choosenP = bus.pileID;
    while (bus.pathTmp[curNode] != -1)
    {
        int edgeID = bus.pathTmp[curNode];  // 存储于edge数组中真正的边的ID

        bus.path.push_back(edgeID / 2); // edgeID / 2是为了适应题目要求
        edge[edgeID].Pile[choosenP] = bus.busID;
        ++edge[edgeID].usedPileCnt;

        if (edgeID % 2) {   // 奇数-1
            edge[edgeID - 1].Pile[choosenP] = bus.busID;   // 双向边，两边一起处理
            ++edge[edgeID - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeID + 1].Pile[choosenP] = bus.busID;
            ++edge[edgeID + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    std::reverse(bus.path.begin(), bus.path.end());
}

// 根据 expectedAllocationPressure 对加载业务的顺序进行排序
void Solution::sortBus()
{
    sortedBusIndices.resize(T);
    for (int i = 0; i < T; ++i)
        sortedBusIndices[i] = i;
    if (!forSortBus)
        return;
    // 根据 expectedAllocationPressure 进行排序
    sort(sortedBusIndices.begin(), sortedBusIndices.end(), [&](int a, int b) {
        // 匿名 lambda 函数作为比较函数
        return [&](int a, int b) {
            // 如果 a 和 b 满足相邻条件，则优先满足相邻条件
            if ((buses[a].start == buses[b].start && buses[a].end == buses[b].end) || (buses[a].start == buses[b].end && buses[a].end == buses[b].start))
                return false; // 保持相邻
            // 否则按照 expectedAllocationPressure 进行降序排序
            return buses[a].expectedAllocationPressure > buses[b].expectedAllocationPressure;
            }(a, b);
        });
}
