#include "UtilityFunction.h"
#include "Configure.h"
#include "global_var.h"
#include "global_struct.h"
#include <iostream>
#include <fstream>

// �ӱߺ�����s��㣬t�յ㣬d����
void addEdge(int s, int t, int d)
{
    edge[cntEdge].from = s; // ���
    edge[cntEdge].to = t;   // �յ�
    edge[cntEdge].d = 1;    // ����
    //edge[cntEdge].d = d;    // ����
    edge[cntEdge].trueD = d;    // ����
    edge[cntEdge].next = head[s];   // ��ʽǰ����sΪ�����һ���ߵı�ţ�head[s]������ǵ�ǰ��sΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
    for (int i = 0; i < P; ++i)
        edge[cntEdge].Pile[i] = -1;
    edge[cntEdge].usedPileCnt = 0;
    head[s] = cntEdge++;    // ������sΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
}

// ��ҵ����
void addBus(int start, int end)
{
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    buses[cntBus].curA = D;
    vector<int>().swap(buses[cntBus].path);
    vector<int>().swap(buses[cntBus].pathTmp);
    vector<int>().swap(buses[cntBus].mutiplierId);
    ++cntBus;
}

// ��׼������������������
void inputFromJudger()
{
    std::cin >> N >> M >> T >> P >> D;
    for (int i = 0; i < N; ++i)
    {   // ��ʼ��һЩ����
        head[i] = -1;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        std::cin >> s >> t >> d;
        if (minDist.find(make_pair(s, t)) == minDist.end()) { // ��������
            minDist[make_pair(s, t)] = Configure::INF;
            minDist[make_pair(t, s)] = Configure::INF;
        }
        addEdge(s, t, d);
        addEdge(t, s, d);   // ���˫���
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        std::cin >> Sj >> Tj;
        addBus(Sj, Tj); // ���ҵ��
    }
}

// �ļ��������������ļ�
void inputFromFile()
{
    std::ifstream myCin("././MATLAB/dataMATLAB.txt"); // ���ļ�
    if (!cin)
    {
        std::cerr << "Error: Cannot open the file." << std::endl;
    }
    myCin >> N >> M >> T >> P >> D;
    for (int i = 0; i < N; ++i)
    {   // ��ʼ��һЩ����
        head[i] = -1;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        myCin >> s >> t >> d;
        if (minDist.find(make_pair(s, t)) == minDist.end()) { // ��������
            minDist[make_pair(s, t)] = Configure::INF;
            minDist[make_pair(t, s)] = Configure::INF;
        }
        addEdge(s, t, d);
        addEdge(t, s, d);   // ���˫���
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        myCin >> Sj >> Tj;
        addBus(Sj, Tj); // ���ҵ��
    }
}

// ��׼�����������������
void outputForJudger()
{
    unordered_map<int, int> newEdgeMap;
    int n = newEdge.size();
    if (n > 0)
    {
        newEdgeMap[newEdgePathId[0]] = M;
        for (int i = 1; i < n; ++i)
        {
            newEdgeMap[newEdgePathId[i]] = newEdgeMap[newEdgePathId[i - 1]] + 1;
        }
    }

    std::cout << newEdge.size() << endl;
    for (int i = 0; i < newEdge.size(); ++i)
    {
        std::cout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();

        std::cout << buses[i].pileId << " " << pSize << " " << mSize << " ";
        for (int j = 0; j < pSize; ++j)
        {
            int pathID = buses[i].path[j];
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
            std::cout << buses[i].mutiplierId[j];
            if (j < buses[i].mutiplierId.size() - 1)
                std::cout << " ";
            else if (j == mSize - 1 && i != T - 1)
                std::cout << endl;
        }
    }
}

// �ļ�������������ļ�
void outputForFile()
{
    ofstream myCout("./MATLAB/result.txt");
    if (!myCout.is_open())
    {
        std::cerr << "Error opening file." << endl;
    }
    unordered_map<int, int> newEdgeMap;
    int n = newEdge.size();
    if (n > 0)
    {
        newEdgeMap[newEdgePathId[0]] = M;
        for (int i = 1; i < n; ++i)
        {
            newEdgeMap[newEdgePathId[i]] = newEdgeMap[newEdgePathId[i - 1]] + 1;
        }
    }

    myCout << newEdge.size() << endl;
    for (int i = 0; i < newEdge.size(); ++i)
    {
        myCout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    int totP = 0, totM = 0;
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();
        totP += pSize;
        totM += mSize;

        myCout << buses[i].pileId << " " << pSize << " " << mSize << " ";
        for (int j = 0; j < pSize; ++j)
        {
            int pathID = buses[i].path[j];
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
            myCout << buses[i].mutiplierId[j];
            if (j < buses[i].mutiplierId.size() - 1)
                myCout << " ";
            else if (j == mSize - 1 && i != T - 1)
                myCout << endl;
        }
    }
    std::cerr << "Total Cost = " << n * 1000000 + totM * 100 + totP;
}
