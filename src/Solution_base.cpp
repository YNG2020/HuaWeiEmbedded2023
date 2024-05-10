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
    for (int i = 0; i < T; ++i)
        BFS_tranStatistic(trans[i]);    // 先找到最短路径及其长度，并统计每条边的使用次数（edge[edgeId].statisticCnt）
    if (Configure::forStatisticOutput && !Configure::forJudger)
        outputStatistic();
    sortedTranIndices.resize(T);
    for (int i = 0; i < T; ++i)
        sortedTranIndices[i] = i;
    
    sumUptheAllocationPressure();     // 求和分配光业务时，每条光业务的期望分配压力
    sortTran();             // 根据 expectedAllocationPressure 对加载业务的顺序进行排序
    resetEverything();      // 把加载光业务对网络的影响全部清除
    preAllocateTran();      // 初分配

    //sumUptheAllocationPressure();
    //sortTran();
    //resetEverything();
    //preAllocateTran();

    forTryDeleteEdge = true;
    tryDeleteEdge();
    tryDeleteEdge();

    ifLast = false;
    if (forIter)
    {
        if (Configure::forIterOutput && !Configure::forJudger)
            std::cout << "Original newEdge.size = " << newEdge.size() << endl;
        for (int cnt = 0; cnt < cntLimit; ++cnt)
        {
            double reallocateTranNum = 0.3 * T;
            //double reallocateTranNum = pow(reAllocateTranNumFunBase, reAllocateTranNumFunExpRatio * cnt) * T;
            reAllocateTran(reallocateTranNum);
            //tryDeleteEdge();
            if (Configure::forIterOutput && !Configure::forJudger)
            {
                std::cout << "Number of Trans to be reallocate: " << reallocateTranNum / T << "T" << endl;
                std::cout << "newEdge.size = " << newEdge.size() << endl;
            }
        }

        tryDeleteEdge();
        tryDeleteEdge();
    }

    if (Configure::forIterOutput && !Configure::forJudger)
        std::cout << "newEdge.size = " << newEdge.size() << endl;
}

// 对业务路径在网络上的分布做一个统计（不考虑通道编号限制）
void Solution::sumUptheAllocationPressure()
{
    for (int i = 0; i < T; ++i)
    {
        Transaction& tran = trans[i];
        tran.expectedAllocationPressure = tran.path.size();
        //tran.expectedAllocationPressure = (tran.path.size() + tran.path.size() - minPathSize[make_pair(tran.start, tran.end)]);
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
    int gap;
    if (strategy == 0)
        gap = 1;       // (TODO，gap的机理需要被弄清楚)
    else
        gap = max(int(0.025 * T), 30);
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
        for (int j = 0; j < gap; ++j)
        {
            tranIDx[j] = totTranIDx[j];
        }

        int oriUsedEdgeNum = 0, oriNewEdgeNum = newEdge.size();

        vector<vector<int>> pathTmp1(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp1(gap, -1);
        for (int j = 0, tranID; j < gap; ++j)
        {
            tranID = tranIDx[j];
            oriUsedEdgeNum += trans[tranID].path.size();
            pathTmp1[j] = trans[tranID].lastEdgesOfShortestPaths;
            pileTmp1[j] = trans[tranID].pileID;
            recoverNetwork(tranID, trans[tranID].pileID);
        }

        int curUsedEdgeNum = 0;
        bool findPath = false;
        vector<vector<int>> pathTmp2(gap, vector<int>());     // 用于此后重新加载边
        vector<int> pileTmp2(gap, -1);
        for (int j = gap - 1, tranID; j >= 0; --j)      // 反向加载效果更好
        {
            tranID = tranIDx[j];
            loadTran(tranID, false);
            pathTmp2[j] = trans[tranID].lastEdgesOfShortestPaths;
            pileTmp2[j] = trans[tranID].pileID;
            curUsedEdgeNum += trans[tranID].path.size();
        }

        int newEdgeNumAftertryDeleteEdge = tryDeleteEdgeSim();

        if (oriNewEdgeNum > newEdgeNumAftertryDeleteEdge)
        {   // 新增的边数减少，接受迁移
            tryDeleteEdge();
            continue;
        }
        else if (oriNewEdgeNum == newEdgeNumAftertryDeleteEdge && oriUsedEdgeNum > curUsedEdgeNum)
        {   // 新增的边数不变，但是使用的边数减少，接受迁移
			tryDeleteEdge();
			continue;
        }
        else {  // 否则，回复原状态
            for (int j = 0, tranID; j < gap; ++j)
            {   // 把试图寻路时，造成的对网络的影响消除
                tranID = tranIDx[j];
                recoverNetwork(tranID, pileTmp2[j]);
            }

            for (int j = i, tranID; j < i + gap; ++j)
            {   // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                tranID = tranIDx[j - i];
                trans[tranID].mutiplierID.swap(nullVector);
                trans[tranID].path.swap(nullPath1);
                trans[tranID].lastEdgesOfShortestPaths.swap(nullPath2);

                trans[tranID].pileID = -1;
                trans[tranID].curA = D;
                reloadTran(tranID, pileTmp1[j - i], pathTmp1[j - i]);
            }
            tryDeleteEdge(false);
        }
    }
}

// 把业务tranID加载到光网络中
void Solution::loadTran(int tranID, bool ifTryDeleteEdge)
{
    BFS_loadTran(trans[tranID], ifTryDeleteEdge); // 加载业务到光网络中
    loadMultiplier(tranID);                      // 加载放大器到业务上
}

// 清空原来的业务对网络的影响
void Solution::recoverNetwork(int tranID, int pileID)
{
    ///////////////////////////////////////////////////////////////////////
    // 清空对寻路的影响
    vector<int> lastEdgesOfShortestPaths = trans[tranID].lastEdgesOfShortestPaths;
    int curNode = trans[tranID].end;
    while (lastEdgesOfShortestPaths[curNode] != -1)
    {
        int edgeID = lastEdgesOfShortestPaths[curNode];  // 存储于edge数组中真正的边的ID
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

        curNode = edge[lastEdgesOfShortestPaths[curNode]].from;
    }

    //////////////////////////////////////////////////////////////////////
    vector<int> nullVector, nullPath1, nullPath2;
    trans[tranID].mutiplierID.swap(nullVector);
    trans[tranID].path.swap(nullPath1);
    trans[tranID].lastEdgesOfShortestPaths.swap(nullPath2);

    trans[tranID].pileID = -1;
    trans[tranID].curA = D;
}

// 重新将某单条业务加载到光网络上
void Solution::reloadTran(int tranID, int pileID, vector<int>& lastEdgesOfShortestPaths)
{
    // 重新设置路径
    trans[tranID].lastEdgesOfShortestPaths = vector<int>(lastEdgesOfShortestPaths.begin(), lastEdgesOfShortestPaths.end());

    int curNode = trans[tranID].end;
    trans[tranID].pileID = pileID;
    while (trans[tranID].lastEdgesOfShortestPaths[curNode] != -1) {
        int edgeID = trans[tranID].lastEdgesOfShortestPaths[curNode];     // 存储于edge数组中真正的边的ID

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
    while (tran.lastEdgesOfShortestPaths[curNode] != -1)
    {
        int edgeID = tran.lastEdgesOfShortestPaths[curNode];  // 存储于edge数组中真正的边的ID

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

        curNode = edge[tran.lastEdgesOfShortestPaths[curNode]].from;
    }
    std::reverse(tran.path.begin(), tran.path.end());
}

// 根据 expectedAllocationPressure 对加载业务的顺序进行排序
void Solution::sortTran()
{   // 在业务分配的后期，加边是一定要加的。应该思考，在需要加边时，如何优化加载业务的顺序，使得加边的数目最少。
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

// 把加载光业务对网络的影响全部清除
void Solution::resetEverything()
{
    for (int i = 0; i < cntEdge; ++i)
    {
        edge[i].usedPileCnt = 0;
        for (int j = 0; j < P; ++j)
            edge[i].Pile[j] = -1;
    }
    for (int i = 0; i < cntTran; ++i)
    {
        trans[i].expectedAllocationPressure = 0;
        trans[i].path.clear();
        trans[i].mutiplierID.clear();
        trans[i].curA = D;
        trans[i].pileID = -1;
        trans[i].lastEdgesOfShortestPaths.clear();
    }
    newEdge.clear();
    newEdgePathID.clear();
    for (int i = 0; i < M; ++i)
    {
        head[i] = oriHead[i];
    }
    for (int i = 0; i < cntEdge; ++i)
    {
        edge[i] = oriEdge[i];
    }
    cntEdge = oriCntEdge;
}

// 在重边之间迁移业务
void Solution::transferTranInMultiEdge(Transaction& tran)
{
    if (!ifLast)
        return;
    for (int i = 0; i < tran.path.size(); ++i)
    {
        int trueEdgeID = tran.path[i] * 2;
        if (trueEdgeID < M * 2)     // 不是新加的边，直接跳过
            continue;
        int from = edge[trueEdgeID].from, to = edge[trueEdgeID].to;
        pair<int, int> nodePair = make_pair(from, to);
        int multiEdgeid = find(multiEdgeID[nodePair].begin(), multiEdgeID[nodePair].end(), trueEdgeID) - multiEdgeID[nodePair].begin();
        int lastEdgeID = multiEdgeID[nodePair][multiEdgeid];
        for (int j = multiEdgeid - 1; j >= 0; --j)
        {   // 尽可能把业务迁移到重边编号小的边上
            int edgeID = multiEdgeID[nodePair][0];
            if (edge[edgeID].Pile[tran.pileID] == -1)
            {
                edge[edgeID].Pile[tran.pileID] = tran.tranID;
                ++edge[edgeID].usedPileCnt;
                edge[edgeID + 1].Pile[tran.pileID] = tran.tranID;
                ++edge[edgeID + 1].usedPileCnt;

                edge[lastEdgeID].Pile[tran.pileID] = -1;
                --edge[lastEdgeID].usedPileCnt;
                edge[lastEdgeID + 1].Pile[tran.pileID] = -1;
                --edge[lastEdgeID + 1].usedPileCnt;
                
                tran.path[i] = edgeID / 2;
                for (int k = 0; k < N; ++k)
                {
                    if (edge[tran.lastEdgesOfShortestPaths[k]].to == edge[edgeID].to && edge[tran.lastEdgesOfShortestPaths[k]].from == edge[edgeID].from)
                    {
                        tran.lastEdgesOfShortestPaths[k] = edgeID;
						break;
                    }
                    if (edge[tran.lastEdgesOfShortestPaths[k]].to == edge[edgeID + 1].to && edge[tran.lastEdgesOfShortestPaths[k]].from == edge[edgeID + 1].from)
                    {
                        tran.lastEdgesOfShortestPaths[k] = edgeID + 1;
						break;
                    }
                }
                lastEdgeID = edgeID;
            }
        }
    }
}
