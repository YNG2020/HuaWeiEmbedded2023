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

// 试图重新分配业务到光网络中
void Solution::reAllocateTran(int HLim)
{
    int gap;
    if (!forBatchTranReAllocate)
        gap = 1;
    else
        gap = int(reAllocateRatio * T);
    ifIterSuccess = false;
    vector<int> totTranIDx(T, 0);
    vector<int> tranIDx(gap, 0);
    for (int i = 0; i < T; ++i)
        totTranIDx[i] = i;
    //std::mt19937 rng(42); // 设置随机数种子  
    //random_shuffle(totTranIDx.begin(), totTranIDx.end());   // 先打乱totTranIDx
    for (int i = 0; i + gap < HLim; i = i + gap)
    {
        getReallocatedTranID(totTranIDx, tranIDx, gap, i);
        for (int j = 0; j < gap; ++j)
        {
            reallocatedTranSta[iterCnter] += (trans[tranIDx[j]].path.size() - minPathSize[make_pair(trans[tranIDx[j]].start, trans[tranIDx[j]].end)]);
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

        bool ifSuccess = tryDeleteEdgeSim(oriNewEdgeNum, oriUsedEdgeNum, curUsedEdgeNum);

        if (!ifSuccess)
        {  // 回复为原状态
            for (int j = 0, tranID; j < gap; ++j)
            {   // 把试图寻路时，造成的对网络的影响消除
                tranID = tranIDx[j];
                recoverNetwork(tranID, pileTmp2[j]);
            }

            for (int j = 0, tranID; j < gap; ++j)
            {   // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                tranID = tranIDx[j];
                trans[tranID].mutiplierID.swap(nullVector);
                trans[tranID].path.swap(nullPath1);
                trans[tranID].lastEdgesOfShortestPaths.swap(nullPath2);

                trans[tranID].pileID = -1;
                trans[tranID].curA = D;
                reloadTran(tranID, pileTmp1[j], pathTmp1[j]);
            }
            tryDeleteEdge(false);
            recordIterSuccess[iterCnter++] = false;
        }
        else
        {
            ifIterSuccess = true;
            recordIterSuccess[iterCnter++] = true;
        }
    }
}

// 获取重新分配的业务ID
void Solution::getReallocatedTranID(vector<int>& totTranIDx, vector<int>& tranIDx, int gap, int gapStart)
{
    std::mt19937 rng(42); // 设置随机数种子  
    random_shuffle(totTranIDx.begin(), totTranIDx.end());   // 先打乱totTranIDx
	//int cnter = 0;
 //   // 定义范围 [0, 1) 的均匀分布
 //   std::uniform_real_distribution<> dis(0.0, 1.0);
 //   int j = 0;
 //   for (; j < gap && cnter < T; ++cnter)
 //   {
 //       // 生成一个 0 到 1 之间的随机小数
 //       double random_number = dis(rng);
 //       int extraEdgeNum = trans[totTranIDx[cnter]].path.size() - 
 //           minPathSize[make_pair(trans[totTranIDx[cnter]].start, trans[totTranIDx[cnter]].end)];
 //       double rto = extraEdgeNum / double(totUsedEdge - minTotUsedEdge);
 //       if (rto * gap * 0.2 > random_number)
	//	    tranIDx[j++] = totTranIDx[cnter];
	//}
 //   cnter = 0;
 //   for (; j < gap && cnter < T; ++cnter)
 //   {
 //       int extraEdgeNum = trans[totTranIDx[cnter]].path.size() -
 //           minPathSize[make_pair(trans[totTranIDx[cnter]].start, trans[totTranIDx[cnter]].end)];
 //       if (extraEdgeNum == 0)
 //           tranIDx[j++] = totTranIDx[cnter];
 //   }
    for (int j = 0; j < gap; ++j)
    {
        tranIDx[j] = totTranIDx[j];
    }
}