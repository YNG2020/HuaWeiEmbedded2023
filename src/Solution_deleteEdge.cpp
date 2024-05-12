#include "Solution.h"
#include "UtilityFunction.h"
#include "global_var.h"
#include "global_struct.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <vector>
#include <random>

// 试图删除新边，参数increasing用于控制删边时，是从旧边开始删，还是从新边开始删
void Solution::tryDeleteEdge(bool increasing, bool ifSimDeleteEdge)
{
    if (!forTryDeleteEdge)
        return;
    int n = newEdge.size(), trueEdgeID;
    vector<int> oriNewEdgePathID = newEdgePathID;
    for (int idx = increasing ? 0 : n - 1; increasing ? (idx < n) : (idx >= 0); increasing ? ++idx : --idx)
    {
        int idxEdge = oriNewEdgePathID[idx]; // idxEdge为边在边集数组的编号（计数时，双向边视作同一边）  

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
        if (tranCnt != 0 && !increasing)
			return;

        vector<vector<int>> lastEdgesOfShortestPaths(tranCnt, vector<int>());     // 用于此后重新加载边
        for (int k = 0; k < tranCnt; ++k)
        {
            lastEdgesOfShortestPaths[k] = trans[lastTranIDs[k]].lastEdgesOfShortestPaths;
            recoverNetwork(lastTranIDs[k], lastPileIDs[k]);
        }

        bool findPath = true;
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
        {   // 成功删除新边，将原本新添加的边idxEdge从newEdge中删除
            performDeleteEdge(idxEdge, tranCnt, lastTranIDs);
        }
        else
        {
            for (int k = 0; k < stopK; ++k)
            {   // 把试图寻路(BFS_detectPath)时，造成的对网络的影响消除
                recoverNetwork(lastTranIDs[k], tmpLastPileIDs[k]);
            }

            for (int k = 0; k < tranCnt; ++k)
            {   // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                trans[lastTranIDs[k]].mutiplierID.swap(nullVector);
                trans[lastTranIDs[k]].path.swap(nullPath1);
                trans[lastTranIDs[k]].lastEdgesOfShortestPaths.swap(nullPath2);

                trans[lastTranIDs[k]].pileID = -1;
                trans[lastTranIDs[k]].curA = D;
                reloadTran(lastTranIDs[k], lastPileIDs[k], lastEdgesOfShortestPaths[k]);
            }
        }
    }
}

// tryDeleteEdge的模拟，参数increasing用于控制删边时，是从旧边开始删，还是从新边开始删
bool Solution::tryDeleteEdgeSim(int oriNewEdgeNum, int oriUsedEdgeNum, int curUsedEdgeNum, bool increasing)
{
    int n = newEdge.size(), trueEdgeID;
    vector<int> oriNewEdgePathID = newEdgePathID;
    int nDeleteEdge = 0;
    vector<vector<int>> totLastTranID, totLastPileID, totTmpLastPileIDs;    // 第一个维度代表新边被删除的顺序，用于此后重新加载边
    vector<vector<vector<int>>> totLastEdgesOfShortestPaths;                // 第一个维度代表新边被删除的顺序，
    vector<int> totDeleteEdgeID;                                            // 用于记录被删除的边的编号

    for (int idx = increasing ? 0 : n - 1; increasing ? (idx < n) : (idx >= 0); increasing ? ++idx : --idx)
    {
        int idxEdge = oriNewEdgePathID[idx]; // idxEdge为边在边集数组的编号（计数时，双向边视作同一边）  

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

        vector<vector<int>> lastEdgesOfShortestPaths(tranCnt, vector<int>());     // 用于此后重新加载边
        for (int k = 0; k < tranCnt; ++k)
        {
            lastEdgesOfShortestPaths[k] = trans[lastTranIDs[k]].lastEdgesOfShortestPaths;
            recoverNetwork(lastTranIDs[k], lastPileIDs[k]);
        }

        bool findPath = true;
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
        {   // 可以删除新边
            for (int k = 0; k < P; ++k)
            {   // 该边已删除，暂时对其进行封锁
                edge[trueEdgeID].Pile[k] = T;
                edge[trueEdgeID + 1].Pile[k] = T;   // 偶数+1
            }
            // 备份
            totLastTranID.push_back(lastTranIDs);
            totLastPileID.push_back(lastPileIDs);
            totLastEdgesOfShortestPaths.push_back(lastEdgesOfShortestPaths);
            totTmpLastPileIDs.push_back(tmpLastPileIDs);
            totDeleteEdgeID.push_back(trueEdgeID);
            ++nDeleteEdge;
        }
        else
        {
            for (int k = 0; k < stopK; ++k)
            {   // 把试图寻路(BFS_detectPathSim)时，造成的对网络的影响消除
                recoverNetwork(lastTranIDs[k], tmpLastPileIDs[k]);
            }

            for (int k = 0; k < tranCnt; ++k)
            {   // 重新加载所有的边
                vector<int> nullVector, nullPath1, nullPath2;
                trans[lastTranIDs[k]].mutiplierID.swap(nullVector);
                trans[lastTranIDs[k]].path.swap(nullPath1);
                trans[lastTranIDs[k]].lastEdgesOfShortestPaths.swap(nullPath2);

                trans[lastTranIDs[k]].pileID = -1;
                trans[lastTranIDs[k]].curA = D;
                reloadTran(lastTranIDs[k], lastPileIDs[k], lastEdgesOfShortestPaths[k]);
            }
        }
    }

    if (oriNewEdgeNum > n - nDeleteEdge)
    {   // 新增的边数减少，接受迁移
        for (int i = nDeleteEdge - 1; i >= 0; --i)
        {   // 将原本新添加的边idxEdge从newEdge中删除
            performDeleteEdge(totDeleteEdgeID[i] / 2, totLastTranID[i].size(), totLastTranID[i]);
        }
        return true;
    }
    else if (oriNewEdgeNum == (n - nDeleteEdge) && oriUsedEdgeNum > curUsedEdgeNum)
    {   // 新增的边数不变，但是使用的边数减少，接受迁移
        for (int i = nDeleteEdge - 1; i >= 0; --i)
        {   // 将原本新添加的边idxEdge从newEdge中删除
            performDeleteEdge(totDeleteEdgeID[i] / 2, totLastTranID[i].size(), totLastTranID[i]);
        }
        return true;;
    }
    else
    {
        // 回溯，将网络和业务恢复到删边前的状态
        for (int i = nDeleteEdge - 1; i >= 0; --i)
        {
            trueEdgeID = totDeleteEdgeID[i];
            for (int j = 0; j < P; ++j)
            {
                edge[trueEdgeID].Pile[j] = -1;
                edge[trueEdgeID + 1].Pile[j] = -1;
            }
            // 遍历totLastPileID[i]，把edge[trueEdgeID]和edge[trueEdgeID+1]的通道状态还原
            int arrSize = totLastPileID[i].size();
            for (int j = 0; j < arrSize; ++j)
            {
                edge[trueEdgeID].Pile[totLastPileID[i][j]] = totLastTranID[i][j];
                edge[trueEdgeID + 1].Pile[totLastPileID[i][j]] = totLastTranID[i][j];
            }
            // 遍历totLastTranID[i]，把试图寻路(BFS_detectPath)时，造成的对网络的影响消除
            arrSize = totLastTranID[i].size();
            for (int j = 0; j < arrSize; ++j)
            {
                recoverNetwork(totLastTranID[i][j], totTmpLastPileIDs[i][j]);
            }
            // 遍历totLastTranID[i]，重新加载所有的边
            for (int j = 0; j < arrSize; ++j)
            {
                vector<int> nullVector, nullPath1, nullPath2;
                trans[totLastTranID[i][j]].mutiplierID.swap(nullVector);
                trans[totLastTranID[i][j]].path.swap(nullPath1);
                trans[totLastTranID[i][j]].lastEdgesOfShortestPaths.swap(nullPath2);

                trans[totLastTranID[i][j]].pileID = -1;
                trans[totLastTranID[i][j]].curA = D;
                reloadTran(totLastTranID[i][j], totLastPileID[i][j], totLastEdgesOfShortestPaths[i][j]);
            }
        }
        return false;
    }
}

// 执行删除边操作
void Solution::performDeleteEdge(int idxEdge, int tranCnt, const vector<int>& lastTranIDs)
{
    int trueEdgeID = idxEdge * 2;

    // 将原本新添加的边从newEdge中删除
    int iter = find(newEdgePathID.begin(), newEdgePathID.end(), idxEdge) - newEdgePathID.begin();
    newEdge.erase(newEdge.begin() + iter);
    newEdgePathID.erase(newEdgePathID.begin() + iter);
    pair<int, int> nodePair = make_pair(edge[trueEdgeID].from, edge[trueEdgeID].to);
    iter = find(multiEdgeID[nodePair].begin(), multiEdgeID[nodePair].end(), trueEdgeID) - multiEdgeID[nodePair].begin();
    multiEdgeID[nodePair].erase(multiEdgeID[nodePair].begin() + iter);

    for (int k = 0; k < tranCnt; ++k)
    {   // 重新加载被迁移的业务的路径上的放大器设置
        vector<int> nullVector;
        trans[lastTranIDs[k]].mutiplierID.swap(nullVector);
        trans[lastTranIDs[k]].curA = D;
        loadMultiplier(lastTranIDs[k]);
    }

    deleteEdge(trueEdgeID);
    deleteEdge(trueEdgeID + 1);

    for (int k = 0; k < P; ++k)
    {   // 该边已删除，就应对其进行封锁
        edge[trueEdgeID].Pile[k] = T;
        edge[trueEdgeID + 1].Pile[k] = T;   // 偶数+1
    }
}
