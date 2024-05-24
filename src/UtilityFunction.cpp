#include "UtilityFunction.h"
#include "Configure.h"
#include "global_var.h"
#include "global_struct.h"
#include <iostream>
#include <fstream>
#include <string>
#include  <stdio.h> 

// 加边函数，s起点，t终点，d距离
void addEdge(int s, int t, int d)
{
    edge[cntEdge].edgeID = cntEdge;  // 边的编号
    edge[cntEdge].from = s; // 起点
    edge[cntEdge].to = t;   // 终点
    edge[cntEdge].d = 1;    // 距离
    //edge[cntEdge].d = d;    // 距离
    edge[cntEdge].trueD = d;    // 距离
    if (head[s] != -1)      // 此前的最后一条以s为起点的新加进来的边在边集数组的位置
        edge[head[s]].pre = cntEdge;
    edge[cntEdge].next = head[s];   // 链式前向。以s为起点下一条边的编号，head[s]代表的是当前以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    for (int i = 0; i < P; ++i)
        edge[cntEdge].Pile[i] = -1;
    edge[cntEdge].usedPileCnt = 0;
    if (tail[s] == -1)
        tail[s] = cntEdge;
    head[s] = cntEdge++;    // 更新以s为起点的在逻辑上的第一条边在边集数组的位置（编号）
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
    if (s > t)
    {
        if (cntEdge % 2)
            multiEdgeID[make_pair(t, s)].push_back(cntEdge - 1);
        else
            multiEdgeID[make_pair(t, s)].push_back(cntEdge - 2);
    }
}

// 删边函数
void deleteEdge(int edgeID)
{
    int pre = edge[edgeID].pre, next = edge[edgeID].next;
    if (pre != -1)
		edge[pre].next = next;
	else // 说明要删除的边是在链表中以点edge[edgeID].from为起点的第一条边，此时要更新链表的头
		head[edge[edgeID].from] = next;
    if (next != -1)
        edge[next].pre = pre;
    if (edgeID == cntEdge - 1)
    {   // 说明要删除的边是最后一条边，尽可能回收利用被删边在边集数组中占用的空间
        --cntEdge;
    }
}

// 加业务函数
void addTran(int start, int end)
{
    trans[cntTran].start = start;
    trans[cntTran].end = end;
    trans[cntTran].tranID = cntTran;
    trans[cntTran].curA = D;
    vector<int>().swap(trans[cntTran].path);
    vector<int>().swap(trans[cntTran].lastEdgesOfShortestPaths);
    vector<int>().swap(trans[cntTran].mutiplierID);
    ++cntTran;
}

// 标准输入流，用于判题器
void inputFromJudger()
{
    std::cin >> N >> M >> T >> P >> D;
    for (int i = 0; i < N; ++i)
    {   // 初始化一些东西
        head[i] = -1;
        tail[i] = -1;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        std::cin >> s >> t >> d;
        if (minDist.find(make_pair(s, t)) == minDist.end()) { // 键不存在
            minDist[make_pair(s, t)] = Configure::INF;
            minDist[make_pair(t, s)] = Configure::INF;
        }
        addEdge(s, t, d);
        addEdge(t, s, d);   // 添加双向边
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        std::cin >> Sj >> Tj;
        addTran(Sj, Tj); // 添加业务
    }
    for (int i = 0; i < M; ++i)
    {
        oriHead[i] = head[i];
        oriTail[i] = tail[i];
    }
    for (int i = 0; i < cntEdge; ++i)
    {
		oriEdge[i] = edge[i];
    }
    oriCntEdge = cntEdge;
    oriMultiEdgeID = multiEdgeID;
}

// 文件输入流，用于文件
void inputFromFile()
{
    //std::ifstream myCin("././MATLAB/dataMATLAB.txt"); // Linux or Win的cmd下
    std::ifstream myCin("dataMATLAB.txt"); // MATLAB 调试下
    if (!myCin)
    {
        std::cerr << "Error: Cannot open the file." << std::endl;
    }
    myCin >> N >> M >> T >> P >> D;
    for (int i = 0; i < N; ++i)
    {   // 初始化一些东西
        head[i] = -1;
        tail[i] = -1;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        myCin >> s >> t >> d;
        if (minDist.find(make_pair(s, t)) == minDist.end()) { // 键不存在
            minDist[make_pair(s, t)] = Configure::INF;
            minDist[make_pair(t, s)] = Configure::INF;
        }
        addEdge(s, t, d);
        addEdge(t, s, d);   // 添加双向边
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        myCin >> Sj >> Tj;
        addTran(Sj, Tj); // 添加业务
    }
    for (int i = 0; i < M; ++i)
    {
        oriHead[i] = head[i];
        oriTail[i] = tail[i];
    }
    for (int i = 0; i < cntEdge; ++i)
    {
        oriEdge[i] = edge[i];
    }
    oriCntEdge = cntEdge;
    oriMultiEdgeID = multiEdgeID;
}

// 标准输出流，用于判题器
void outputForJudger()
{
    unordered_map<int, int> newEdgeMap;
    int n = newEdge.size();
    if (n > 0)
    {
        newEdgeMap[newEdgePathID[0]] = M;
        for (int i = 1; i < n; ++i)
        {
            newEdgeMap[newEdgePathID[i]] = newEdgeMap[newEdgePathID[i - 1]] + 1;
        }
    }

    std::cout << newEdge.size() << endl;
    for (int i = 0; i < newEdge.size(); ++i)
    {
        std::cout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = trans[i].path.size();
        int mSize = trans[i].mutiplierID.size();

        std::cout << trans[i].pileID << " " << pSize << " " << mSize << " ";
        for (int j = 0; j < pSize; ++j)
        {
            int pathID = trans[i].path[j];
            if (pathID >= M)
                pathID = newEdgeMap[pathID];

            std::cout << pathID;
            if (mSize == 0 && j == pSize - 1 && i != T - 1)
            {
                std::cout << endl;
            }
            else if (mSize != 0 || j != pSize - 1)
            {
                std::cout << " ";
            }
        }
        for (int j = 0; j < mSize; ++j)
        {
            std::cout << trans[i].mutiplierID[j];
            if (j < trans[i].mutiplierID.size() - 1)
                std::cout << " ";
            else if (j == mSize - 1 && i != T - 1)
                std::cout << endl;
        }
    }
    totCost = n;
}

// 文件输出流，用于文件
void outputForFile()
{
    ofstream myCout("./output/" + dataGenSeed + "result.txt");
    if (!myCout.is_open())
    {
        std::cerr << "Error opening file." << endl;
    }
    unordered_map<int, int> newEdgeMap;
    int n = newEdge.size();
    if (n > 0)
    {
        newEdgeMap[newEdgePathID[0]] = M;
        for (int i = 1; i < n; ++i)
        {
            newEdgeMap[newEdgePathID[i]] = newEdgeMap[newEdgePathID[i - 1]] + 1;
        }
    }

    myCout << newEdge.size() << endl;
    for (int i = 0; i < newEdge.size(); ++i)
    {
        myCout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    int totP = 0, totM = 0;
    for (int i = 0; i < T; ++i) {
        int pSize = trans[i].path.size();
        int mSize = trans[i].mutiplierID.size();
        totP += pSize;
        totM += mSize;

        myCout << trans[i].pileID << " " << pSize << " " << mSize << " ";
        for (int j = 0; j < pSize; ++j)
        {
            int pathID = trans[i].path[j];
            if (pathID >= M)
                pathID = newEdgeMap[pathID];

            myCout << pathID;
            if (mSize == 0 && j == pSize - 1 && i != T - 1)
            {
                myCout << endl;
            }
            else if (mSize != 0 || j != pSize - 1)
            {
                myCout << " ";
            }
        }
        for (int j = 0; j < mSize; ++j)
        {
            myCout << trans[i].mutiplierID[j];
            if (j < trans[i].mutiplierID.size() - 1)
                myCout << " ";
            else if (j == mSize - 1 && i != T - 1)
                myCout << endl;
        }
    }
    myCout.close();
    totCost = n;
    totCost = n * 1000000 + totM * 100 + totP;
    std::cerr << "Total Cost = " << totCost;
    // 以下输出边集数组的每条边（原有边和新加入的边）的通道分配情况
    ofstream myCout1("./output/" + dataGenSeed + string("transactionInPile.txt"));
    if (!myCout1.is_open())
    {
        return;
    }
    for (int i = 0; i < M; ++i)
    {
        myCout1 << edge[i * 2].from << " " << edge[i * 2].to << " " << edge[i * 2].usedPileCnt << " ";
        for (int j = 0; j < P; ++j)
        {
            myCout1 << edge[i * 2].Pile[j] << " ";
        }
        myCout1 << endl;
    }
    for (int i = 0; i < newEdge.size(); ++i)
    {
        myCout1 << edge[newEdgePathID[i] * 2].from << " " << edge[newEdgePathID[i] * 2].to << " " << edge[newEdgePathID[i] * 2].usedPileCnt << " ";
        for (int j = 0; j < P; ++j)
        {
			myCout1 << edge[newEdgePathID[i] * 2].Pile[j] << " ";
		}
		myCout1 << endl;
	}
    myCout1.close();

    ofstream myCout2("./output/" + dataGenSeed + string("iterStatistic.txt"));
    for (int i = 0; i < recordIterSuccess.size(); ++i)
    {
		myCout2 << recordIterSuccess[i] << " " << reallocatedTranSta[i] << " " <<
            recordIterNewEdgeNum[i] << " " << recordIterTotUsedPile[i] << " " << endl;
	}
}

// 输出业务在网络上的分布的统计结果
void outputStatistic()
{
	ofstream myCout("transactionStatistic.txt");
    if (!myCout.is_open())
    {
        return;
	}
    for (int i = 0; i < M; ++i)
    {
        myCout << edge[i * 2].from << " " << edge[i * 2].to << " " << edge[i * 2].usedPileCnt << endl;
	}
	myCout.close();

    ofstream myCout1("transactionMinPath.txt");
    if (!myCout1.is_open())
    {
        return;
    }
    for (int i = 0; i < T; ++i)
    {
        int pSize = trans[i].path.size();
        myCout1 << pSize << " ";
        for (int j = 0; j < pSize; ++j)
        {
            int pathID = trans[i].path[j];

            myCout1 << pathID;
            if (j == pSize - 1 && i != T - 1)
            {
                myCout1 << endl;
            }
            else if (j != pSize - 1)
            {
                myCout1 << " ";
            }
        }
    }
    myCout1.close();
}

// 成本计算函数
void calculateCost()
{
    int totP = 0, totM = 0;
    for (int i = 0; i < T; ++i)
    {
        int pSize = trans[i].path.size();
        int mSize = trans[i].mutiplierID.size();
        totP += pSize;
        totM += mSize;
    }
    totCost = newEdge.size() * 1000000 + totM * 100 + totP;
}

// 统计所有业务上用掉的边的数量
void sumUPAllUsedEdge()
{
    totUsedPile = 0;
    for (int i = 0; i < T; ++i)
    {
        totUsedPile += trans[i].path.size();
    }
}
