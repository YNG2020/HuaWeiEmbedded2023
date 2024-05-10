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
void Solution::tryDeleteEdge(bool increasing)
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

// 执行删除边操作
void Solution::performDeleteEdge(int idxEdge, int tranCnt, const vector<int>& lastTranIDs)
{
    // 将原本新添加的边从newEdge中删除
    int iter = find(newEdgePathID.begin(), newEdgePathID.end(), idxEdge) - newEdgePathID.begin();
    newEdge.erase(newEdge.begin() + iter);
    newEdgePathID.erase(newEdgePathID.begin() + iter);
    int trueEdgeID = idxEdge * 2;
    pair<int, int> nodePair = make_pair(edge[trueEdgeID].from, edge[trueEdgeID].to);
    iter = find(multiEdgeID[nodePair].begin(), multiEdgeID[nodePair].end(), trueEdgeID) - multiEdgeID[nodePair].begin();
    multiEdgeID[nodePair].erase(multiEdgeID[nodePair].begin() + iter);

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
