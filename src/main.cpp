#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <math.h>
#include <time.h>
#include <cmath>
#include <fstream>
#include "struct.h"
#include "configure.h"
const int INF = 2147483647;

using namespace std;

int N, M, T, P, D; // �ڵ�����N����������M��ҵ������T������ͨ������P�����˥������D
int cntEdge = 0;    // ��ǰ�߼��������洢�ıߵ���Ŀ
int cntBus = 0;     // ��ǰҵ���������洢ҵ�����Ŀ

int head[configure::maxN]; // head[i]����ʾ��iΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
bool vis1[configure::maxN];  // ��ʶ�õ����ޱ����ʹ�
vector<pair<int, int>> newEdge; // ��¼����ӵıߵ������յ�
vector<int> newEdgePathId;   // ��¼�±��ڱ߼��е�λ�ã�����ʱ��˫�����Ϊͬһ�ߣ�
vector<int> remainBus;  // ��¼�³��η���ʱ����·���������޷�����ߵ�ҵ��ı��

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void BFS_loadBus(Business& bus, bool ifLoadNewEdge);
bool BFS_detectPath(Business& bus, int blockEdge);
void BFS_addNewEdge(Business& bus);
void loadBus(int busId, bool ifLoadRemain);
void loadMultiplier(int busId);
void allocateBus();
void reAllocateBus(int HLim);
void tryDeleteEdge();
void reverseArray(vector<int>& arr);
void outPut();
void reCoverNetwork(int lastBusID, int lastPileId);
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);

bool ifLast = false;
bool ifTryDeleteEdge = true;

// ������
int main() {

    //std::ifstream cin("dataMATLAB.txt"); // ���ļ�
    //if (!cin) {
    //    std::cerr << "Error: Cannot open the file." << std::endl;
    //    return 1;
    //}

    cin >> N >> M >> T >> P >> D;
    init();
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {
        cin >> s >> t >> d;
        if (minDist.find(make_pair(s, t)) == minDist.end()) { // ��������
            minDist[make_pair(s, t)] = INF;
            minDist[make_pair(t, s)] = INF;
        }
        addEdge(s, t, d);
        addEdge(t, s, d);   // ���˫���
    }

    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        cin >> Sj >> Tj;
        addBus(Sj, Tj); // ���ҵ��
    }

    allocateBus();

    ifTryDeleteEdge = false;

    int cnt = 0;

    if (!configure::isUseProblemFinder)
        std::cout << "Original newEdge.size" << newEdge.size() << endl;
    while (cnt < configure::cntLimit) {

        reAllocateBus(pow(2.71, -0.005 * cnt)*T);
        tryDeleteEdge();
        if (!configure::isUseProblemFinder)
            std::cout << "newEdge.size" << newEdge.size() << endl;
        cnt = cnt + 1;
    }

    ifLast = true;
    tryDeleteEdge();
    tryDeleteEdge();
    if (!configure::isUseProblemFinder)
        std::cout << "newEdge.size" << newEdge.size() << endl;
    outPut();

    return 0;
}

// ��ʼ��
void init() {

    for (int i = 0; i < N; ++i) {

        head[i] = -1;
        node[i].NodeId = i;
        for (int j = 0; j < P; ++j)
            node[i].Multiplier[j] = -1;
    }

    for (int i = 0; i < T; ++i) {
        buses[i].curA = D;
    }

}

// �����е�ҵ����䵽��������
void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i, false);
    }
}

// ��ͼ���·���ҵ�񵽹�������
void reAllocateBus(int HLim) {

    int gap = max(int(0.025 * T), 20);
    if (gap > T)
        return;
    vector<int> totBusIdx(T, 0);
    vector<int> busIdx(gap, 0);
    for (int i = 0; i < T; ++i)
        totBusIdx[i] = i;

    for (int i = 0; i + gap < HLim; i = i + gap) {

        srand(1);  // �������������  
        random_shuffle(totBusIdx.begin(), totBusIdx.end());
        for (int i = 0; i < gap; ++i) {
            busIdx[i] = totBusIdx[i];
        }

        int oriEdgeNum = 0;

        vector<vector<int>> pathTmp1(gap, vector<int>());     // ���ڴ˺����¼��ر�
        vector<int> pileTmp1(gap, -1);
        for (int j = i, busId; j < i + gap; ++j) {
            busId = busIdx[j - i];
            oriEdgeNum += buses[busId].path.size();
            pathTmp1[j - i] = buses[busId].pathTmp;
            pileTmp1[j - i] = buses[busId].pileId;
            reCoverNetwork(busId, buses[busId].pileId);
        }

        int curEdgeNum = 0;
        bool findPath = false;
        vector<vector<int>> pathTmp2(gap, vector<int>());     // ���ڴ˺����¼��ر�
        vector<int> pileTmp2(gap, -1);
        for (int j = i + gap - 1, busId; j >= i; --j) {
            busId = busIdx[j - i];
            loadBus(busId, false);
            pathTmp2[j - i] = buses[busId].pathTmp;
            pileTmp2[j - i] = buses[busId].pileId;
            curEdgeNum += buses[busId].path.size();
        }

        if (oriEdgeNum > curEdgeNum) {  // ����ı������٣�����Ǩ��
            continue;
        }
        else {  // ���򣬻ظ�ԭ״̬
            for (int j = i + gap - 1, busId; j >= i; --j) {   // ����ͼѰ·ʱ����ɵĶ������Ӱ������
                busId = busIdx[j - i];
                reCoverNetwork(busId, pileTmp2[j - i]);
            }

            for (int j = i, busId; j < i + gap; ++j) {  // ���¼������еı�
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

// ��ͼɾ���±�
void tryDeleteEdge() {

    int n = newEdge.size(), trueEdgeId;
    for (int idx = 0; idx < n; ++idx) {
        int idxEdge = newEdgePathId[idx]; // idxEdgeΪ���ڱ߼�����ı�ţ�����ʱ��˫�������ͬһ�ߣ�  

        trueEdgeId = idxEdge * 2;
        int busCnt = 0;
        vector<int> lastBusIds, lastPileIds;

        for (int j = 0; j < P; ++j)
            if (edge[trueEdgeId].Pile[j] != -1 && edge[trueEdgeId].Pile[j] != T) {   // ˵����ͨ��j�ϳ����˸�ҵ��
                ++busCnt;
                lastBusIds.push_back(edge[trueEdgeId].Pile[j]);
                lastPileIds.push_back(j);
            }

        if (busCnt == 0) {      // ������±��ϣ�һ��ҵ��û�г��أ�ֱ��ɾ��

            int iter = find(newEdgePathId.begin(), newEdgePathId.end(), idxEdge) - newEdgePathId.begin();
            newEdge.erase(newEdge.begin() + iter);
            newEdgePathId.erase(newEdgePathId.begin() + iter);
            --idx;
            --n;

            for (int k = 0; k < P; ++k) {   // �ñ���ɾ������Ӧ������з���
                edge[trueEdgeId].Pile[k] = T;
                edge[trueEdgeId + 1].Pile[k] = T;   // ż��+1
            }
        }
        else {  // ������±��ϣ������˶���ҵ����Ըñ��ϵ�����ҵ�����·��䣬���ܷ��������±�

            vector<vector<int>> pathTmp(busCnt, vector<int>());     // ���ڴ˺����¼��ر�
            for (int k = 0; k < busCnt; ++k) {
                pathTmp[k] = buses[lastBusIds[k]].pathTmp;
                reCoverNetwork(lastBusIds[k], lastPileIds[k]);
            }

            bool findPath = false;
            int stopK = -1;
            vector<int> tmpLastPileIds;
            for (int k = 0; k < busCnt; ++k) {

                findPath = BFS_detectPath(buses[lastBusIds[k]], idxEdge);

                if (findPath == false) {
                    stopK = k;
                    break;
                }
                tmpLastPileIds.push_back(buses[lastBusIds[k]].pileId);   // ԭ����pileId�Ѹı䣬�˴����и��£��Է�ֹreCoverNetworkʱ��bug
            }

            if (findPath) {

                int iter = find(newEdgePathId.begin(), newEdgePathId.end(), idxEdge) - newEdgePathId.begin();
                newEdge.erase(newEdge.begin() + iter);
                newEdgePathId.erase(newEdgePathId.begin() + iter);
                --idx;
                --n;

                for (int k = 0; k < P; ++k) {   // �ñ���ɾ������Ӧ������з���
                    edge[trueEdgeId].Pile[k] = T;
                    edge[trueEdgeId + 1].Pile[k] = T;   // ż��+1
                }

                for (int k = 0; k < busCnt; ++k) {

                    vector<int> nullVector;
                    buses[lastBusIds[k]].mutiplierId.swap(nullVector);
                    buses[lastBusIds[k]].curA = D;

                    int curNode = buses[lastBusIds[k]].start, trueNextEdgeId;
                    for (int i = 0; i < buses[lastBusIds[k]].path.size(); ++i) {

                        if (edge[buses[lastBusIds[k]].path[i] * 2].from == curNode)
                            trueNextEdgeId = buses[lastBusIds[k]].path[i] * 2;
                        else
                            trueNextEdgeId = buses[lastBusIds[k]].path[i] * 2 + 1;
                        curNode = edge[trueNextEdgeId].to;

                        if (buses[lastBusIds[k]].curA >= edge[trueNextEdgeId].trueD) {
                            buses[lastBusIds[k]].curA -= edge[trueNextEdgeId].trueD;
                        }
                        else {
                            node[edge[trueNextEdgeId].from].Multiplier[buses[lastBusIds[k]].pileId] = buses[lastBusIds[k]].pileId;
                            buses[lastBusIds[k]].curA = D;
                            buses[lastBusIds[k]].curA -= edge[trueNextEdgeId].trueD;
                            buses[lastBusIds[k]].mutiplierId.push_back(edge[trueNextEdgeId].from);
                        }
                    }

                }

            }
            else {

                for (int k
                    = 0; k < stopK; ++k) {   // ����ͼѰ·ʱ����ɵĶ������Ӱ������
                    reCoverNetwork(lastBusIds[k], tmpLastPileIds[k]);
                }

                for (int k = 0; k < busCnt; ++k) {  // ���¼������еı�
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

// ��ҵ��busId���ص���������
void loadBus(int busId, bool ifLoadRemain) {

    BFS_loadBus(buses[busId], false);
    loadMultiplier(busId);

}

// �ӱߺ�����s��㣬t�յ㣬d����
void addEdge(int s, int t, int d) {
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
void addBus(int start, int end) {
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    vector<int>().swap(buses[cntBus].path);
    vector<int>().swap(buses[cntBus].pathTmp);
    vector<int>().swap(buses[cntBus].mutiplierId);
    ++cntBus;
}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������ٱ���·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void BFS_loadBus(Business& bus, bool ifLoadNewEdge) {

    int start = bus.start, end = bus.end;
    static int addNewEdgeCnt = 0;  // �ӱߴ��������Ǳ�����
    static int addNewBus = 0;   // ��ҵ�����
    ++addNewBus;
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    double maxValue = -1;

    for (int p = 0; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        vis1[start] = true;
        int s = start;
        int t = -1;
        int curLevel = 0;

        while (!bfsQ.empty() && t != end) { // ����Ϊ�ռ������е㶼�����뵽��������ȥ��

            s = bfsQ.front().first;
            curLevel = bfsQ.front().second;
            bfsQ.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i

                    if (t == end) {
                        ++curLevel;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (t == end) {

            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // ��������
                minPathSize[make_pair(start, end)] = curLevel;
            else if (minPathSize[make_pair(start, end)] > curLevel) {
                minPathSize[make_pair(start, end)] = curLevel;
            }

            if (ifLoadNewEdge) {    // ���BFS1�ڵ���ǰ�Ѿ�������±ߣ������һ���
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
                findPath = true;
                break;
            }

            if (curLevel > 3 * minPathSize[make_pair(start, end)])  // �ҵ���·������̫������Ը��Ҫ
                continue;

            int curNode = end, tmpDist = curLevel;
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // �Ҳ���·����Ҫ�����±�

        if (ifTryDeleteEdge) {
            if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))   // ���ƹ���ɾ�ߣ�ɾ����ʱ�ϳ���
                tryDeleteEdge();
        }
        BFS_addNewEdge(bus);       // �¼ӱ߲��ԣ�ֻ�����·������Ҫ���мӱߵı�
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id

        bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // ����-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // ż��+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[edgeId].from;
    }
    reverseArray(bus.path);
}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������������Ҫ�ӱߵ��������������ֱ�ӷ���
bool BFS_detectPath(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (int p = 0; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) { // ����Ϊ�ռ������е㶼�����뵽��������ȥ��

            s = bfsQ.front().first;
            curLevel = bfsQ.front().second;
            bfsQ.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    int t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i

                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        ++curLevel;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }

                }
            }

        }
        if (s == end) {

            // �ҵ���·������̫������Ը��Ҫ
            if (!ifLast && (curLevel > 3 * minPathSize[make_pair(start, end)]))
                continue;

            int curNode = end, tmpDist = curLevel;
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // �Ҳ���·����Ҫ�����±�
        return false;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id

        bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
        edge[edgeId].Pile[choosenP] = bus.busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // ����-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // ż��+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);
    return true;
}

// ���ܼ���ҵ��ʱ��Ѱ��ҵ��bus����㵽�յ��·����������ͨ��������ȫͨ��������������·���ϵķ��������ı߽��мӱߴ���
void BFS_addNewEdge(Business& bus) {

    int start = bus.start, end = bus.end;
    int minBlockEdge = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    tmpOKPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        vis1[i] = false;
    }
    queue<pair<int, int>> bfsQ;
    bfsQ.push(make_pair(start, 0));
    vis1[start] = true;
    int s = start;
    int curLevel = 0;
    bool getOutFlag = false;

    while (!bfsQ.empty() && !getOutFlag) {

        s = bfsQ.front().first;
        curLevel = bfsQ.front().second;
        bfsQ.pop();

        for (int i = head[s]; i != -1; i = edge[i].next) {

            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i

            if (t == end) {
                getOutFlag = true;
                s = t;
                ++curLevel;
                break;
            }
            else
                bfsQ.push(make_pair(t, curLevel + 1));
        }

    }
    for (int p = 0; p < P; ++p) {

        int curNode = end, tmpBlockEdge = 0;
        while (tmpOKPath[curNode] != -1) {
            int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
            if (edge[edgeId].Pile[p] != -1)
                ++tmpBlockEdge;
            curNode = edge[edgeId].from;
        }

        if (tmpBlockEdge < minBlockEdge) {   // ѡ��Ҫ�ӱ������ٵ�ͨ��
            minBlockEdge = tmpBlockEdge;
            bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            choosenP = p;
        }

    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id
        int lastNode = curNode;
        curNode = edge[bus.pathTmp[curNode]].from;

        if (edge[edgeId].Pile[choosenP] == -1) {    // ����ӱ�
            bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
            edge[edgeId].Pile[choosenP] = bus.busId;
            ++edge[edgeId].usedPileCnt;

            if (edgeId % 2) {   // ����-1
                edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
                ++edge[edgeId - 1].usedPileCnt;
            }
            else {  // ż��+1
                edge[edgeId + 1].Pile[choosenP] = bus.busId;
                ++edge[edgeId + 1].usedPileCnt;
            }
        }
        else {      // ��Ҫ�ӱ�
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
            newEdgePathId.emplace_back(cntEdge / 2 - 1);

            bus.path.push_back(cntEdge / 2 - 1); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
            edge[cntEdge - 2].Pile[choosenP] = bus.busId;
            ++edge[cntEdge - 2].usedPileCnt;
            edge[cntEdge - 1].Pile[choosenP] = bus.busId;   // ż��+1
            ++edge[cntEdge - 1].usedPileCnt;
            bus.pathTmp[lastNode] = cntEdge - 2;
        }

    }
    reverseArray(bus.path);
}

// ��ת����
void reverseArray(vector<int>& arr) {

    int tmp, n = arr.size();
    for (int i = 0; i < n / 2; ++i) {
        tmp = arr[n - i - 1];
        arr[n - i - 1] = arr[i];
        arr[i] = tmp;
    }

}

// ��������
void outPut() {

    ofstream cout("result.txt");
    if (!cout.is_open()) {
        cout << "Error opening file." << endl;
    }
    unordered_map<int, int> newEdgeMap;
    int n = newEdge.size();
    if (n > 0) {
        newEdgeMap[newEdgePathId[0]] = M;
        for (int i = 1; i < n; ++i) {
            newEdgeMap[newEdgePathId[i]] = newEdgeMap[newEdgePathId[i - 1]] + 1;
        }
    }

    cout << newEdge.size() << endl;
    for (int i = 0; i < newEdge.size(); ++i) {
        cout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();

        cout << buses[i].pileId << " " << pSize << " "
            << mSize << " ";
        for (int j = 0; j < pSize; ++j) {

            int pathID = buses[i].path[j];
            if (pathID >= M)
                pathID = newEdgeMap[pathID];

            cout << pathID;
            if (mSize == 0 && j == pSize - 1 && i != T - 1) {
                cout << endl;
            }
            else if (mSize != 0 || j != pSize - 1) {
                cout << " ";
            }
        }
        for (int j = 0; j < mSize; ++j) {
            cout << buses[i].mutiplierId[j];
            if (j < buses[i].mutiplierId.size() - 1)
                cout << " ";
            else if (j == mSize - 1 && i != T - 1)
                cout << endl;
        }
    }
}

// �����ԭ����ҵ��������Ӱ��
void reCoverNetwork(int lastBusID, int lastPileId) {

    ///////////////////////////////////////////////////////////////////////
    // ��ն�Ѱ·��Ӱ��
    vector<int> pathTmp = buses[lastBusID].pathTmp;
    int curNode = buses[lastBusID].end;
    while (pathTmp[curNode] != -1) {
        int edgeId = pathTmp[curNode];  // �洢��edge�����������ıߵ�Id
        edge[edgeId].Pile[lastPileId] = -1;
        --edge[edgeId].usedPileCnt;

        if (edgeId % 2) { // ����-1
            edge[edgeId - 1].Pile[lastPileId] = -1;   // ˫��ߣ�����һ����
            --edge[edgeId - 1].usedPileCnt;
        }
        else {  // ż��+1
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

// ���¼���ҵ��������
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp) {

    // ��������·��   
    int curNode = buses[lastBusID].end;
    buses[lastBusID].pileId = lastPileId;
    buses[lastBusID].pathTmp = vector<int>(pathTmp.begin(), pathTmp.end());
    while (buses[lastBusID].pathTmp[curNode] != -1) {
        int edgeId = buses[lastBusID].pathTmp[curNode];  // �洢��edge�����������ıߵ�Id

        buses[lastBusID].path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
        edge[edgeId].Pile[lastPileId] = buses[lastBusID].busId;
        ++edge[edgeId].usedPileCnt;

        if (edgeId % 2) {   // ����-1
            edge[edgeId - 1].Pile[lastPileId] = buses[lastBusID].busId;   // ˫��ߣ�����һ����
            ++edge[edgeId - 1].usedPileCnt;
        }
        else {  // ż��+1
            edge[edgeId + 1].Pile[lastPileId] = buses[lastBusID].busId;
            ++edge[edgeId + 1].usedPileCnt;
        }

        curNode = edge[buses[lastBusID].pathTmp[curNode]].from;
    }
    reverseArray(buses[lastBusID].path);

    // �������÷Ŵ���
    loadMultiplier(lastBusID);
}

// ���طŴ�����ҵ����
void loadMultiplier(int busId) {
    int curNode = buses[busId].start, trueNextEdgeId;
    for (int i = 0; i < buses[busId].path.size(); ++i) {

        if (edge[buses[busId].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[busId].path[i] * 2;
        else
            trueNextEdgeId = buses[busId].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[busId].curA >= edge[trueNextEdgeId].trueD) {
            buses[busId].curA -= edge[trueNextEdgeId].trueD;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[busId].pileId] = buses[busId].pileId;
            buses[busId].curA = D;
            buses[busId].curA -= edge[trueNextEdgeId].trueD;
            buses[busId].mutiplierId.push_back(edge[trueNextEdgeId].from);
        }
    }
}