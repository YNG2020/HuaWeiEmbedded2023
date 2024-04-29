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
    sortTran();  // 根据 expectedAllocationPressure 对加载业务的顺序进行排序
    preAllocateTran();
    tryDeleteEdge();

    if (forIter)
    {
        if (Configure::forIterOutput && !Configure::forJudger)
            std::cout << "Original newEdge.size = " << newEdge.size() << endl;
        for (int cnt = 0; cnt < cntLimit; ++cnt)
        {
            reAllocateTran(pow(reAllocateTranNumFunBase, reAllocateTranNumFunExpRatio * cnt) * T);
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
		BFS_tranStatistic(trans[i]);
    for (int i = 0; i < T; ++i)
    {
        Transaction& tran = trans[i];
        int pathSize = tran.pathStatistic.size();
        for (int j = 0; j < pathSize; ++j)
        {
            tran.expectedAllocationPressure += (edge[tran.pathStatistic[j] * 2].statisticCnt % P);
        }
        tran.expectedAllocationPressure = tran.pathStatistic.size();
    }
}

// 将所有的业务分配到光网络中
void Solution::preAllocateTran()
{
    for (int i = 0; i < T; ++i)
    {
        loadTran(sortedTranIndices[i], true);
    }
}

// 试图重新分配业务到光网络中
void Solution::reAllocateTran(int HLim)
{
    int gap = max(int(0.025 * T), 20);       // (TODO，gap的机理需要被弄清楚)
    if (gap > T)
        return;
    vector<int> totTranIDx(T, 0);
    vector<int> tranIDx(gap, 0);
    for (int i = 0; i < T; ++i)
        totTranIDx[i] = i;

    for (int i = 0; i + gap < HLim; i = i + gap)
    {
        std::mt19937 rng(42); // 设置随机数种子  
        random_shuffle(totTranIDx.begin(), totTranIDx.end());
        for (int i = 0; i < gap; ++i)
        {
            tranIDx[i] = totTranIDx[i];
        }

        int oriEdgeNum = 0;

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = i, tranID; j < i + gap; ++j)
        {
            tranID = tranIDx[j - i];
            oriEdgeNum += trans[tranID].path.size();
            pathTmp1[j - i] = trans[tranID].pathTmp;
            pileTmp1[j - i] = trans[tranID].pileID;
            recoverNetwork(tranID, trans[tranID].pileID);
        }

        int curEdgeNum = 0;
        bool findPath = false;
        vector<vector<int>> pathTmp2(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, tranID; j >= i; --j)
        {
            tranID = tranIDx[j - i];
            loadTran(tranID, false);
            pathTmp2[j - i] = trans[tranID].pathTmp;
            pileTmp2[j - i] = trans[tranID].pileID;
            curEdgeNum += trans[tranID].path.size();
        }

        if (oriEdgeNum > curEdgeNum)
        {   // 总体的边数减少，接受迁移
            continue;
        }
        else {  // 否则，回复原状态
            for (int j = i + gap - 1, tranID; j >= i; --j)
            {   // 把试图寻路时，造成的对网络的影响消除
                tranID = tranIDx[j - i];
                recoverNetwork(tranID, pileTmp2[j - i]);
            }

            for (int j = i, tranID; j < i + gap; ++j)
            {   // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                tranID = tranIDx[j - i];
                trans[tranID].mutiplierID.swap(nullVector);
                trans[tranID].path.swap(nullPath1);
                trans[tranID].pathTmp.swap(nullPath2);

                trans[tranID].pileID = -1;
                trans[tranID].curA = D;
                reloadTran(tranID, pileTmp1[j - i], pathTmp1[j - i]);
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
        int tranCnt = 0;
        vector<int> lastTranIDs, lastPileIDs;        // 存储将要被删除的边上承载的业务ID和通道ID

        for (int j = 0; j < P; ++j)
            if (edge[trueEdgeID].Pile[j] != -1 && edge[trueEdgeID].Pile[j] != T)
            {   // 说明在通道j上承载了该业务
                ++tranCnt;
                lastTranIDs.push_back(edge[trueEdgeID].Pile[j]);
                lastPileIDs.push_back(j);
            }

        if (tranCnt == 0) 
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
            vector<vector<int>> pathTmp(tranCnt, vector<int>());     // 用于此后重新加载边
            for (int k = 0; k < tranCnt; ++k)
            {
                pathTmp[k] = trans[lastTranIDs[k]].pathTmp;
                recoverNetwork(lastTranIDs[k], lastPileIDs[k]);
            }

            bool findPath = false;
            int stopK = -1;
            vector<int> tmpLastPileIDs;                                 // 作用见该变量被使用时对应的注释
            for (int k = 0; k < tranCnt; ++k)
            {
                findPath = BFS_detectPath(trans[lastTranIDs[k]], idxEdge);
                if (findPath == false)
                {
                    stopK = k;
                    break;
                }
                tmpLastPileIDs.push_back(trans[lastTranIDs[k]].pileID);   // 原本的pileID已改变，此处进行更新，以防止reCoverNetwork时出bug
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

                for (int k = 0; k < tranCnt; ++k)
                {   // 重新加载被迁移的业务的路径上的放大器设置
                    vector<int> nullVector;
                    trans[lastTranIDs[k]].mutiplierID.swap(nullVector);
                    trans[lastTranIDs[k]].curA = D;
                    loadMultiplier(lastTranIDs[k]);
                }
            }
            else
            {
                for (int k = 0; k < stopK; ++k)
                {   // 把试图寻路时，造成的对网络的影响消除
                    recoverNetwork(lastTranIDs[k], tmpLastPileIDs[k]);
                }

                for (int k = 0; k < tranCnt; ++k)
                {   // 重新加载所有的边
                    vector<int> nullVector, nullPath1, nullPath2;
                    trans[lastTranIDs[k]].mutiplierID.swap(nullVector);
                    trans[lastTranIDs[k]].path.swap(nullPath1);
                    trans[lastTranIDs[k]].pathTmp.swap(nullPath2);

                    trans[lastTranIDs[k]].pileID = -1;
                    trans[lastTranIDs[k]].curA = D;
                    reloadTran(lastTranIDs[k], lastPileIDs[k], pathTmp[k]);
                }
            }
        }
    }
}

// 把业务tranID加载到光网络中
void Solution::loadTran(int tranID, bool ifTryDeleteEdge)
{
    BFS_detectMinPathSize(trans[tranID]);        // 先找到最短路径的长度，以便于后续的路径长度限制
    BFS_loadTran(trans[tranID], ifTryDeleteEdge); // 加载业务到光网络中
    loadMultiplier(tranID);                      // 加载放大器到业务上
}

// 清空原来的业务对网络的影响
void Solution::recoverNetwork(int tranID, int pileID)
{
    ///////////////////////////////////////////////////////////////////////
    // 清空对寻路的影响
    vector<int> pathTmp = trans[tranID].pathTmp;
    int curNode = trans[tranID].end;
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
    trans[tranID].mutiplierID.swap(nullVector);
    trans[tranID].path.swap(nullPath1);
    trans[tranID].pathTmp.swap(nullPath2);

    trans[tranID].pileID = -1;
    trans[tranID].curA = D;
}

// 重新将某单条业务加载到光网络上
void Solution::reloadTran(int tranID, int pileID, vector<int>& pathTmp)
{
    // 重新设置路径
    trans[tranID].pathTmp = vector<int>(pathTmp.begin(), pathTmp.end());

    int curNode = trans[tranID].end;
    trans[tranID].pileID = pileID;
    while (trans[tranID].pathTmp[curNode] != -1) {
        int edgeID = trans[tranID].pathTmp[curNode];     // 存储于edge数组中真正的边的ID

        trans[tranID].path.push_back(edgeID / 2);        // edgeID / 2是为了适应题目要求
        edge[edgeID].Pile[pileID] = trans[tranID].tranID;
        ++edge[edgeID].usedPileCnt;

        if (edgeID % 2) {   // 奇数-1
            edge[edgeID - 1].Pile[pileID] = trans[tranID].tranID;   // 双向边，两边一起处理
            ++edge[edgeID - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeID + 1].Pile[pileID] = trans[tranID].tranID;
            ++edge[edgeID + 1].usedPileCnt;
        }

        curNode = edge[edgeID].from;
    }
    std::reverse(trans[tranID].path.begin(), trans[tranID].path.end());

    // 重新设置放大器
    loadMultiplier(tranID);
}

// 加载放大器到业务上
void Solution::loadMultiplier(int tranID)
{
    int curNode = trans[tranID].start, trueNextEdgeID;
    for (int i = 0; i < trans[tranID].path.size(); ++i)
    {
        if (edge[trans[tranID].path[i] * 2].from == curNode)
            trueNextEdgeID = trans[tranID].path[i] * 2;
        else
            trueNextEdgeID = trans[tranID].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeID].to;

        if (trans[tranID].curA >= edge[trueNextEdgeID].trueD)
        {
            trans[tranID].curA -= edge[trueNextEdgeID].trueD;
        }
        else
        {
            node[edge[trueNextEdgeID].from].Multiplier[trans[tranID].pileID] = trans[tranID].pileID;
            trans[tranID].curA = D;
            trans[tranID].curA -= edge[trueNextEdgeID].trueD;
            trans[tranID].mutiplierID.push_back(edge[trueNextEdgeID].from);
        }
    }
}

// 回溯路径，以构造出一条完整的路径
void Solution::backtrackPath(Transaction& tran)
{
    int curNode = tran.end;
    int choosenP = tran.pileID;
    while (tran.pathTmp[curNode] != -1)
    {
        int edgeID = tran.pathTmp[curNode];  // 存储于edge数组中真正的边的ID

        tran.path.push_back(edgeID / 2); // edgeID / 2是为了适应题目要求
        edge[edgeID].Pile[choosenP] = tran.tranID;
        ++edge[edgeID].usedPileCnt;

        if (edgeID % 2) {   // 奇数-1
            edge[edgeID - 1].Pile[choosenP] = tran.tranID;   // 双向边，两边一起处理
            ++edge[edgeID - 1].usedPileCnt;
        }
        else {  // 偶数+1
            edge[edgeID + 1].Pile[choosenP] = tran.tranID;
            ++edge[edgeID + 1].usedPileCnt;
        }

        curNode = edge[tran.pathTmp[curNode]].from;
    }
    std::reverse(tran.path.begin(), tran.path.end());
}

// 根据 expectedAllocationPressure 对加载业务的顺序进行排序
void Solution::sortTran()
{   // 在业务分配的后期，加边是一定要加的。应该思考，在需要加边时，如何优化加载业务的顺序，使得加边的数目最少。
    sortedTranIndices.resize(T);
    for (int i = 0; i < T; ++i)

        sortedTranIndices[i] = i;
    if (!forSortTran)
        return;
    // 根据 expectedAllocationPressure 进行排序
    sort(sortedTranIndices.begin(), sortedTranIndices.end(), [&](int a, int b) {
        // 匿名 lambda 函数作为比较函数
        return [&](int a, int b) {
            // 如果 a 和 b 满足相邻条件，则优先满足相邻条件
            if ((trans[a].start == trans[b].start && trans[a].end == trans[b].end) || (trans[a].start == trans[b].end && trans[a].end == trans[b].start))
                return false; // 保持相邻
            // 否则按照 expectedAllocationPressure 进行降序排序
            return trans[a].expectedAllocationPressure > trans[b].expectedAllocationPressure;
            }(a, b);
        });
}
