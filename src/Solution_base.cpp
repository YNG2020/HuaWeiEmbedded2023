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
#include <time.h>
#include <chrono>
#include <ctime>

// Solution初始化
Solution::Solution()
{
    tmpOKPath.resize(N);
}

// 光线扩容难题总策略
void Solution::runStrategy()
{  
    clock_t startTime, curTime, reAllocateTime;
    double iterationUnitTime = 0;
    startTime = clock();

    for (int i = 0; i < T; ++i)
        BFS_tranStatistic(trans[i]);    // 先找到最短路径及其长度
    if (Configure::forStatisticOutput && !Configure::forJudger)
        outputStatistic();
    sortedTranIndices.resize(T);
    for (int i = 0; i < T; ++i)
        sortedTranIndices[i] = i;
    
    sumUptheAllocationPressure();     // 求和分配光业务时，每条光业务的期望分配压力
    sortTran();             // 根据 expectedAllocationPressure 对加载业务的顺序进行排序
    //// 输出加载业务的顺序
    //for (int i = 0; i < T; ++i)
    //{
    //    std::cout << "TranID: " << trans[sortedTranIndices[i]].tranID << " start: " << trans[sortedTranIndices[i]].start << "  end: " << trans[sortedTranIndices[i]].end << "  pathSize: " << trans[sortedTranIndices[i]].minPathSize << endl;
    //}
    resetEverything();      // 把加载光业务对网络的影响全部清除
    preAllocateTran();      // 初分配    

    tryDeleteEdge();
    tryDeleteEdge();

    sumUPAllUsedEdge();

    curTime = clock();
    iterCnterLimit = cntLimit * (T / int(reAllocateRatio * T));
    recordIterSuccess.resize(iterCnterLimit, false);
    reallocatedTranSta.resize(iterCnterLimit, 0);
    recordIterNewEdgeNum.resize(iterCnterLimit, 0);
    recordIterTotUsedPile.resize(iterCnterLimit, 0);
    double leftTime = 120 - double(curTime - startTime) / CLOCKS_PER_SEC;

    int cnt = 0;
    if (forIter)
    {
        if (Configure::forIterOutput && !Configure::forJudger)
            std::cout << "Original newEdge.size = " << newEdge.size() << endl;
        for (; cnt < cntLimit && iterCnter < iterCnterLimit; ++cnt)
        {   
            reAllocateTime = clock();
            int oriTotUsedEdge = totUsedPile, oriNewEdge = newEdge.size();
            reAllocateTran(T);
            if (Configure::forIterOutput && !Configure::forJudger)
            {
                std::cout << "epoch: " << cnt;
                std::cout << "  newEdge.size = " << newEdge.size();
                std::cout << "  changeUsedEdge: " << totUsedPile - oriTotUsedEdge;
                std::cout << "  totUsedEdge: " << totUsedPile << endl;
            }

    //        curTime = clock();
    //        iterationUnitTime = double(curTime - reAllocateTime) / CLOCKS_PER_SEC;
    //        leftTime = 120 - double(curTime - startTime) / CLOCKS_PER_SEC;
    //        if (leftTime < iterationUnitTime)
				//break;
        }
    }

    curTime = clock();
    if (!Configure::forJudger)
        std::cout << "  Used Time: " << double(curTime - startTime) / CLOCKS_PER_SEC << "s  ";
    if (!Configure::forJudger)
        std::cout << "Iteration Number: " << cnt << " ";
}

// 对业务路径在网络上的分布做一个统计（不考虑通道编号限制）
void Solution::sumUptheAllocationPressure()
{
    for (int i = 0; i < T; ++i)
    {
        Transaction& tran = trans[i];
        tran.expectedAllocationPressure = tran.path.size();
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
    //// 获取当前时间点
    //auto now = std::chrono::system_clock::now();
    //// 转换为微秒
    //auto micros = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    //// 从时间点中获取距离1970年1月1日的微秒数
    //long long micros_from_epoch = micros.time_since_epoch().count();
    //srand(5);    // 设置随机数种子
    //random_shuffle(sortedTranIndices.begin(), sortedTranIndices.end());   // 先打乱totTranIDx
    if (!forSortTran)
        return;
    // 根据 expectedAllocationPressure 进行排序
    std::sort(sortedTranIndices.begin(), sortedTranIndices.end(), [&](int a, int b) {
        if (trans[a].path.size() != trans[b].path.size())
            return trans[a].path.size() > trans[b].path.size();  // 首先按path.size()降序排序
        int startA = trans[a].start, startB = trans[b].start, endA = trans[a].end, endB = trans[b].end;
        if (startA > endA)
        {
            int temp = startA;
            startA = endA;
            endA = temp;
        }
        if (startB > endB)
		{
			int temp = startB;
			startB = endB;
            endB = temp;
		}
        if (startA != startB)
            return startA > startB;  // 其次按start降序排序
        return endA > endB;  // 最后按end降序排序
    });

    vector<int> sortedTranIndices2(T, 0);
    int sortedTranIndices2Cnt = 0;
    vector<vector<int>> groups;
    groups.push_back(vector<int>());
    int groupIdx = 0;
    int lastGroupIdx = 0;
    groups[groupIdx].push_back(sortedTranIndices[0]);
    for (int j = 1; j < T; ++j)
    {
        if (trans[sortedTranIndices[j]].path.size() != trans[sortedTranIndices[j - 1]].path.size())
        {   // 对相同的pathSize的组，再次进行排序，使得排序后相邻的两个tran的起点或终点尽可能不相同
            int commonCnt = 0;
            bool getoutFlag = false;
            while (!getoutFlag)
            {
                getoutFlag = true;
                for (int idx = lastGroupIdx; idx <= groupIdx; ++idx)
                {
                    if (commonCnt < groups[idx].size())
                    {
                        sortedTranIndices2[sortedTranIndices2Cnt++] = groups[idx][commonCnt];
                        getoutFlag = false;
                    }
                }
                ++commonCnt;
            }
            lastGroupIdx = groupIdx + 1;
        }
        int startA = trans[sortedTranIndices[j - 1]].start, startB = trans[sortedTranIndices[j]].start,
            endA = trans[sortedTranIndices[j - 1]].end, endB = trans[sortedTranIndices[j]].end;
        if (startA > endA)
        {
            int temp = startA;
            startA = endA;
            endA = temp;
        }
        if (startB > endB)
        {
            int temp = startB;
            startB = endB;
            endB = temp;
        }
        // 判断下一个编号是否要加入到当前groupIdx中
        if (startA != startB || endA != endB)
        {
            ++groupIdx;
            groups.push_back(vector<int>());
        }
        groups[groupIdx].push_back(sortedTranIndices[j]);
        if (j == T - 1)
        {   // 特殊处理最后一批的组
            int commonCnt = 0;
            bool getoutFlag = false;
            while (!getoutFlag)
            {
                getoutFlag = true;
                for (int idx = lastGroupIdx; idx <= groupIdx; ++idx)
                {
                    if (commonCnt < groups[idx].size())
                    {
                        sortedTranIndices2[sortedTranIndices2Cnt++] = groups[idx][commonCnt];
                        getoutFlag = false;
                    }
                }
                ++commonCnt;
            }
        }
    }
    sortedTranIndices = sortedTranIndices2;
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
        tail[i] = oriTail[i];
    }
    for (int i = 0; i < cntEdge; ++i)
    {
        edge[i] = oriEdge[i];
    }
    cntEdge = oriCntEdge;
    multiEdgeID = oriMultiEdgeID;
}

// 在重边之间迁移业务
void Solution::transferTranInMultiEdge(Transaction& tran)
{
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
            int edgeID = multiEdgeID[nodePair][j];
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
                {   // 将tran.lastEdgesOfShortestPaths[k]上的边编号替换为edgeID或edgeID+1
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
