#include "generateRandomNet.h"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#define INF 2147483647

using namespace std;

int N, M, T, P, D; // �ڵ�����N����������M��ҵ������T������ͨ������P�����˥������D
const int maxM = 50000; // �ߵ������Ŀ
const int maxN = 5000;  // �ڵ�������Ŀ
const int maxBus = 10000;   // ҵ��������Ŀ
const int maxP = 80;    // ��󵥱�ͨ������P
int cntEdge = 0;    // ��ǰ�߼��������洢�ıߵ���Ŀ
int cntBus = 0;     // ��ǰҵ���������洢ҵ�����Ŀ

class Node {
public:
    int NodeId;           // ʵ��������������±�Ϳ���Ψһ��ʶNode����������һ������
    int Multiplier[maxP]; // �ýڵ��ϴ��ڵķŴ�������¼���ǵ�ǰҪ�Ŵ��ͨ���ı�ţ��Ŵ���������ʱֵΪ-1
    vector<int> reachPile;  // �ڵ���dijkstra�����У��ɵִ�ö����pile���
}node[maxN];

class Edge {
public:
    int from, to, d, next;    // ��㣬�յ㣬�ߵľ��룬ͬ������һ������edge�еı��
    int trueD;    // �ߵ���������
    int Pile[maxP]; // �ñ��ϴ��ڵ�ͨ������¼���ǵ�ǰ���ص�ҵ��ı�ţ�������ҵ��ʱֵΪ-1
    Edge() {
        from = -1;
        to = -1;
        d = 0;
        next = -1;
    }
}edge[maxM];    // �߼�����

class Business {
public:
    int start;  // ҵ�����
    int end;    // ҵ���յ�
    int busId;  // ҵ��Id
    int curA;   // ��ǰ�ź�ǿ��
    Business() {
        start = -1;
        end = -1;
    }
    int pileId; // ҵ����ռ�ݵ�ͨ��Id
    vector<int> pathTmp;   // �洢����㵽����������·����ĩ�ߵı�ţ�����ͨ����������̣�
    vector<int> trueMinPath;   // �洢����㵽����������·����ĩ�ߵı�ţ�������ͨ����������̣�
    vector<int> path;   // �洢·���������ı�
    vector<int> mutiplierId;    // �洢�������ķŴ������ڽڵ�ı��
}buses[maxBus];

int head[maxN]; // head[i]����ʾ��iΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
int dis[maxN];  // dis[i]����ʾ��Դ�㵽i������
bool vis1[maxN];  // ��ʶ�õ����ޱ����ʹ�
bool vis2[maxN]; // ��ʶ�õ����������ĳҵ��ʱ����·���������ʹ�
int minDist2start[maxN]; // ��¼�õ㵽����ڿ���ͨ�������µģ���ͨ���е���С�����룬ֵΪ-1ʱ���õ㲻�ɴֵΪ0�����ʱ��ֵ����õ㵽���ı���
int tmpDist2start[maxN]; // ��¼�õ㵽����ڿ���ͨ�������µģ���ǰͨ���еģ����룬ֵΪ-1ʱ���õ㲻�ɴֵΪ0�����ʱ��ֵ����õ㵽���ı���vector<pair<int, int>> newEdge; // ��¼����ӵıߵ������յ�
vector<pair<int, int>> newEdge; // ��¼����ӵıߵ������յ�

struct HashFunc_t {
    size_t operator() (const pair<int, int>& key) const {
        return hash<int>()(key.first) ^ hash<int>()(key.second);;
    }
};

struct Equalfunc_t {
    bool operator() (pair<int, int> const& a, pair<int, int> const& b) const {
        return a.first == b.first && a.second == b.second;
    }
};

unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;   // ��¼�����ڵ�����̱ߵľ���

class Node1 {
public:
    int d, nodeId;

    Node1(int d, int nodeId) {
        this->d = d;
        this->nodeId = nodeId;
    }

    Node1() {}
    bool operator < (const Node1& x) const { // �������������С��Ԫ�ط��ڶѶ�������ѣ�
        return d > x.d;
    }
};

priority_queue<Node1>q;  // ���ȶ��У��������ʱ�ĵ�nodeId��Դ��ľ����nodeId��ű���
queue<pair<int, int>> bfsQ;

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void dijkstra1(Business& bus);
void dijkstra2(Business& bus);
void dijkstra3(Business& bus);
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath);
void BFS1(Business& bus);
void BFS2(Business& bus);
void BFS3(Business& bus);
void BFS4(int start, int end, int pileId, vector<int>& tmpOKPath);
void loadBus(int busId);
void allocateBus();
void reverseArray(vector<int>& arr);
void outPut();
bool bfsTestConnection(int start, int end);
void findAddPath1(Business& bus, bool* vis2);
void findAddPath2(Business& bus, int* vis2);

int testTime = 0;
int maxTestTime = 10;
vector<vector<int>> rEdge;
vector<vector<int>> rBus;
vector<int> allNewEdgeCnt;
vector<int> allPathCnt;
vector<int> allMultiplierCnt;

//// ������
//int main() {
//
//    cin >> N >> M >> T >> P >> D;
//    init();
//    int s = 0, t = 0, d = 0;
//    for (int i = 0; i < M; ++i) {
//        cin >> s >> t >> d;
//        minDist[make_pair(s, t)] = INF;
//        minDist[make_pair(t, s)] = INF;
//        addEdge(s, t, d);
//        addEdge(t, s, d);   // ���˫���
//    }
//
//    int Sj, Tj;
//    for (int i = 0; i < T; ++i) {
//        cin >> Sj >> Tj;
//        addBus(Sj, Tj); // ���ҵ��
//    }
//
//    allocateBus();
//    outPut();
//
//    return 0;
//}


// ������
int main() {

    allNewEdgeCnt.resize(maxTestTime, 0);
    allPathCnt.resize(maxTestTime, 0);
    allMultiplierCnt.resize(maxTestTime, 0);

    for (; testTime < maxTestTime; ++testTime) {
        cntEdge = 0;    // ��ǰ�߼��������洢�ıߵ���Ŀ
        cntBus = 0;     // ��ǰҵ���������洢ҵ�����Ŀ
        minDist.clear();    // ���
        vector<vector<int>>().swap(rEdge);
        vector<vector<int>>().swap(rBus);
        vector<pair<int, int>>().swap(newEdge);

        generateRandomNet(rEdge, rBus, N, M, T, P, D);

        init();
        int s = 0, t = 0, d = 0;
        for (int i = 0; i < M; ++i) {
            s = rEdge[i][0], t = rEdge[i][1], d = rEdge[i][2];
            minDist[make_pair(s, t)] = INF;
            minDist[make_pair(t, s)] = INF;
            addEdge(s, t, d);
            addEdge(t, s, d);   // ���˫���
        }

        int Sj, Tj;
        for (int i = 0; i < T; ++i) {
            Sj = rBus[i][0], Tj = rBus[i][1];
            addBus(Sj, Tj); // ���ҵ��
        }

        allocateBus();
        outPut();
        cout << "\ntestTime " << testTime << endl;
        cout << "allNewEdgeCnt " << allNewEdgeCnt[testTime] << endl;
        cout << "allPathCnt " << allPathCnt[testTime] << endl;
        cout << "allMultiplierCnt " << allMultiplierCnt[testTime] << endl;
    }

    return 0;
}

void allocateBus() {
    for (int i = 0; i < T; ++i) {
        loadBus(i);
    }
}

void loadBus(int busId) {
    //BFS1(buses[busId]);
    dijkstra1(buses[busId]);
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

void init() {   // ��ʼ��

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

void addEdge(int s, int t, int d) {    // �ӱߺ�����s��㣬t�յ㣬d����
    edge[cntEdge].from = s; // ���
    edge[cntEdge].to = t;   // �յ�
    edge[cntEdge].d = 1;    // ����
    edge[cntEdge].trueD = d;    // ����
    edge[cntEdge].next = head[s];   // ��ʽǰ����sΪ�����һ���ߵı�ţ�head[s]������ǵ�ǰ��sΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
    for (int i = 0; i < P; ++i)
        edge[cntEdge].Pile[i] = -1;

    head[s] = cntEdge++;    // ������sΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
}

void addBus(int start, int end) {   // ��ҵ����
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    vector<int>().swap(buses[cntBus].path);
    vector<int>().swap(buses[cntBus].pathTmp);
    vector<int>().swap(buses[cntBus].mutiplierId);
    
    ++cntBus;
}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������ٱ���·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void BFS1(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    // �����Ż�����ҵ��ļӱ߲��ԣ���ȫ������Ч������
    for (int i = 0; i < N; ++i) {
        minDist2start[i] = INF;
        vector<int>().swap(node[i].reachPile);
    }
    minDist2start[start] = 0;

    for (; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            vis1[i] = false;
        }
        queue<pair<int, int>> null_queue;
        bfsQ.swap(null_queue);
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

                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    int t = edge[i].to;
                    if (vis1[t])
                        continue;
                    vis1[t] = true;
                    tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                    tmpDist2start[t] = curLevel + 1;
                    if (tmpDist2start[t] < minDist2start[t]) {
                        minDist2start[t] = tmpDist2start[t];
                        node[t].reachPile.push_back(p);
                    }
                    if (t == end) {
                        getOutFlag = true;
                        s = t;
                        curLevel += 1;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                    }
                }
            }

        }
        if (s == end) {

            //if (p == 0) {
            //    choosenP = p;
            //    findPath = true;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    break;
            //}
            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // �Ҳ���·����Ҫ�����±ߣ������ṩ�����ֲ��ԣ�ʹ��ʱ��Ҫע�͵�����һ��

        //findAddPath2(bus, dist2start); // �����Ż�����ҵ��ļӱ߲��ԣ���ȫ������Ч������
        BFS2(bus);       // �ɵļӱ߲��ԣ�һ���ӱߣ�����·������ӣ���ȫ�������ǵ�ǰ��õ�
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id

        bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
        edge[edgeId].Pile[choosenP] = bus.busId;

        if (edgeId % 2) // ����-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
        else            // ż��+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;

        curNode = edge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);

}

// Ѱ��ҵ��bus����㵽�յ��·����������ͨ��������������·���ϵ�ÿһ���߶�ִ�мӱ߲�����Ȼ�󽻸�BFS1����
void BFS2(Business& bus) {

    int start = bus.start, end = bus.end;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        vis1[i] = false;
    }

    queue<pair<int, int>> null_queue;
    bfsQ.swap(null_queue);
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
            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            bus.trueMinPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
            if (t == end) {
                getOutFlag = true;
                s = t;
                curLevel += 1;
                break;
            }
            else {
                bfsQ.push(make_pair(t, curLevel + 1));
            }
        }

    }

    int curNode = end;
    while (bus.trueMinPath[curNode] != -1) {
        int edgeId = bus.trueMinPath[curNode];  // �洢��edge�����������ıߵ�Id

        addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
        addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

        if (edge[edgeId].from < edge[edgeId].to)
            newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
        else
            newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);

        curNode = edge[bus.trueMinPath[curNode]].from;
    }
    BFS1(bus);

}

// Ѱ��ҵ��bus����㵽�յ㣬�ڲ�����ͨ�������µ����·��
void BFS3(Business& bus) {

    int start = bus.start, end = bus.end;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        vis1[i] = false;
    }

    queue<pair<int, int>> null_queue;
    bfsQ.swap(null_queue);
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
            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            bus.trueMinPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
            if (t == end) {
                getOutFlag = true;
                s = t;
                curLevel += 1;
                break;
            }
            else {
                bfsQ.push(make_pair(t, curLevel + 1));
            }
        }

    }

}

// Ѱ�Ҵ�start��end�����ض�ͨ���µ����·��
void BFS4(int start, int end, int pileId, vector<int>& tmpOKPath) {

    if (start == end)
        return;
    for (int i = 0; i < N; ++i) { // ����ֵ
        vis1[i] = false;
    }

    tmpOKPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        vis1[i] = false;
    }
    queue<pair<int, int>> null_queue;
    bfsQ.swap(null_queue);
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

            if (edge[i].Pile[pileId] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                int t = edge[i].to;
                if (vis1[t])
                    continue;
                vis1[t] = true;
                tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                if (t == end) {
                    getOutFlag = true;
                    s = t;
                    curLevel += 1;
                    break;
                }
                else {
                    bfsQ.push(make_pair(t, curLevel + 1));
                }
            }
        }

    }

}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void dijkstra1(Business& bus) {

    //dijkstra3(bus);
    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    // �����Ż�����ҵ��ļӱ߲��ԣ���ȫ������Ч������
    for (int i = 0; i < N; ++i) {
        vis2[i] = false;
        minDist2start[i] = INF;
        vector<int>().swap(node[i].reachPile);
    }
    vis2[start] = true;
    minDist2start[start] = 0;

    for (; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            dis[i] = INF;
            vis1[i] = false;
        }
        dis[start] = 0;  // Դ�㵽Դ��ľ���Ϊ0
        priority_queue<Node1> null_queue; // ����һ���յ�priority_queue����
        q.swap(null_queue);
        q.push(Node1(0, start));
        int s = -1;
        while (!q.empty()) {   // ��Ϊ�ռ������е㶼�����뵽��������ȥ��
            Node1 x = q.top();  // ��¼�Ѷ���������С�ıߣ������䵯��
            q.pop();
            s = x.nodeId;   // ��s��dijstra�������ϵĵ㣬Դ�㵽s����̾�����ȷ��

            if (s == end) { // ��end�Ѿ����뵽�������������������
                break;
            }

            // û�б���������Ҫ����
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������

                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    int t = edge[i].to;

                    vis2[t] = true;
                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                        dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                        q.push(Node1(dis[t], t));   // ���±������ĵ�������

                        //vis2[t] = true;
                        if (dis[t] < minDist2start[t]) {
                            minDist2start[t] = dis[t];
                            node[t].reachPile.push_back(p);
                        }

                    }
                }
                else {

                }

            }
        }
        if (s == end) {

            //if (p == 0) {
            //    choosenP = p;
            //    findPath = true;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    break;
            //}
            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

            ////����pͨ����·������ʵ�ϵ����·��ʱ�������p��ѭ��
            //int curNode = end, tmpDist = 0;
            //bool breakFlag = true;
            //while (tmpOKPath[curNode] != -1) {

            //    if (bus.trueMinPath[curNode] != tmpOKPath[curNode]) {   // �����·�����в�ͬ���Ͳ��ж�����һ��ͨ��������
            //        breakFlag = false;
            //    }
            //    int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
            //    curNode = edge[edgeId].from;
            //    tmpDist += edge[edgeId].d;
            //    if (curNode == start)
            //        break;
            //}
            //if (breakFlag) {
            //    choosenP = p;
            //    findPath = true;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    break;
            //}
            //if (tmpDist < minPathDist) {
            //    minPathDist = tmpDist;
            //    bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
            //    choosenP = p;
            //}
            //findPath = true;

        }
    }

    if (findPath == false) {    // �Ҳ���·����Ҫ�����±ߣ������ṩ�����ֲ��ԣ�ʹ��ʱ��Ҫע�͵�����һ��

        findAddPath1(bus, vis2); // �����Ż�����ҵ��ļӱ߲��ԣ���ȫ������Ч������
        //dijkstra2(bus);       // �ɵļӱ߲��ԣ�һ���ӱߣ�����·������ӣ���ȫ�������ǵ�ǰ��õ�
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id

        bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
        edge[edgeId].Pile[choosenP] = bus.busId;

        if (edgeId % 2) // ����-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
        else            // ż��+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;


        curNode = edge[bus.pathTmp[curNode]].from;
    }
    reverseArray(bus.path);
}

// Ѱ��ҵ��bus����㵽�յ��·��������·���ϵ�ÿһ���߶�ִ�мӱ߲�����Ȼ�󽻸�dijkstra1����
void dijkstra2(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // Դ�㵽Դ��ľ���Ϊ0

    priority_queue<Node1> null_queue; // ����һ���յ�priority_queue����
    q.swap(null_queue);// �������������е�����
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // ��Ϊ�ռ������е㶼�����뵽��������ȥ��
        Node1 x = q.top();  // ��¼�Ѷ���������С�ıߣ������䵯��
        q.pop();
        s = x.nodeId;   // ��s��dijstra�������ϵĵ㣬Դ�㵽s����̾�����ȷ��

        if (s == end) { // ��end�Ѿ����뵽�������������������
            bus.pileId = p;
            break;
        }

        // û�б���������Ҫ����
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                bus.trueMinPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                q.push(Node1(dis[t], t));   // ���±������ĵ�������  
            }
        }
    }

    int curNode = end;
    while (bus.trueMinPath[curNode] != -1) {
        int edgeId = bus.trueMinPath[curNode];  // �洢��edge�����������ıߵ�Id

        addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
        addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);

        if (edge[edgeId].from < edge[edgeId].to)
            newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
        else
            newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);


        curNode = edge[bus.trueMinPath[curNode]].from;
    }
    dijkstra1(bus);
    return;
}

// Ѱ��ҵ��bus����㵽�յ㣬�ڲ�����ͨ�������µ����·��
void dijkstra3(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    bus.trueMinPath.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // Դ�㵽Դ��ľ���Ϊ0

    priority_queue<Node1> null_queue; // ����һ���յ�priority_queue����
    q.swap(null_queue);// �������������е�����
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // ��Ϊ�ռ������е㶼�����뵽��������ȥ��
        Node1 x = q.top();  // ��¼�Ѷ���������С�ıߣ������䵯��
        q.pop();
        s = x.nodeId;   // ��s��dijstra�������ϵĵ㣬Դ�㵽s����̾�����ȷ��

        if (s == end) { // ��end�Ѿ����뵽�������������������
            bus.pileId = p;
            break;
        }

        // û�б���������Ҫ����
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                bus.trueMinPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                q.push(Node1(dis[t], t));   // ���±������ĵ�������  
            }
        }
    }
}

// Ѱ�Ҵ�start��end�����ض�ͨ���µ����·��
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath) {

    if (start == end)
        return;
    for (int i = 0; i < N; ++i) { // ����ֵ
        dis[i] = INF;
        vis1[i] = false;
    }
    dis[start] = 0;  // Դ�㵽Դ��ľ���Ϊ0

    priority_queue<Node1> null_queue; // ����һ���յ�priority_queue����
    q.swap(null_queue);// �������������е�����
    q.push(Node1(0, start));
    int s = -1;
    while (!q.empty()) {   // ��Ϊ�ռ������е㶼�����뵽��������ȥ��
        Node1 x = q.top();  // ��¼�Ѷ���������С�ıߣ������䵯��
        q.pop();
        s = x.nodeId;   // ��s��dijstra�������ϵĵ㣬Դ�㵽s����̾�����ȷ��

        if (s == end) { // ��end�Ѿ����뵽�������������������
            break;
        }

        // û�б���������Ҫ����
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������

            if (edge[i].Pile[pileId] == -1) {
                int t = edge[i].to;
                if (dis[t] > dis[s] + edge[i].d) {
                    tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                    dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                    q.push(Node1(dis[t], t));   // ���±������ĵ�������  
                }
            }

        }
    }

}

void reverseArray(vector<int>& arr) {

    int tmp, n = arr.size();
    for (int i = 0; i < n / 2; ++i) {
        tmp = arr[n - i - 1];
        arr[n - i - 1] = arr[i];
        arr[i] = tmp;
    }

}

void outPut() {
    cout << newEdge.size() << endl;

    int totNewEdge = newEdge.size();
    int totPathSize = 0;
    int totMultiplier = 0;

    for (int i = 0; i < newEdge.size(); ++i) {
        cout << newEdge[i].first << " " << newEdge[i].second << endl;
    }
    for (int i = 0; i < T; ++i) {
        int pSize = buses[i].path.size();
        int mSize = buses[i].mutiplierId.size();

        totPathSize += pSize;
        totMultiplier += mSize;

        cout << buses[i].pileId << " " << pSize << " "
            << mSize << " ";
        for (int j = 0; j < pSize; ++j) {
            cout << buses[i].path[j];
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

    allNewEdgeCnt[testTime] = totNewEdge;
    allPathCnt[testTime] = totPathSize;
    allMultiplierCnt[testTime] = totMultiplier;
}

// ����start��end����֮�����ͨ�ԣ���ͨ�򷵻�true
bool bfsTestConnection(int start, int end) {

    if (start == end)
        return true;
    vector<bool> vis3(N, false);
    vis3[start] = true;
    queue<int> q;
    q.push(start);

    while (!q.empty()) {

        int curNode = q.front();
        q.pop();
        for (int i = head[curNode]; i != -1; i = edge[i].next) {
            int t = edge[i].to;
            if (vis3[t])
                continue;
            vis3[t] = true;
            if (t == end)
                return true;
            else {
                q.push(t);
            }
        }

    }
    return false;

}

// ����ͨ����������Ӳ���ҵ��ʱ��Ѱ�Һ��ʵ�λ�ý��мӱ߲��������ٴ�Ѱ·
void findAddPath1(Business& bus, bool* vis2) {

    int end = bus.end;  // end���ڴ�֮ǰ��֤���ɴ�
    vector<bool> vis3(N, false);
    vis3[end] = true;
    queue<int> q;
    q.push(end);
    int firstOKPoint = bus.start;
    int OKPile = -1;
    vector<int> tmpOKPath(N, -1);   // �洢·���������ǵ�ÿһ�����ǰһ���ߵı��

    bool getOutFlag = false;
    while (!q.empty() && !getOutFlag) {    // Ѱ��end����һ���ɴ���·��

        int curNode = q.front();
        q.pop();
        for (int i = head[curNode]; i != -1; i = edge[i].next) {

            int t = edge[i].to;
            if (vis3[t])
                continue;
            vis3[t] = true;
            tmpOKPath[t] = i;

            if (vis2[t]) {

                firstOKPoint = t;
                if (t == bus.start)
                    OKPile = 0;
                else {

                    int curMinDist = minDist2start[t];
                    OKPile = node[t].reachPile[node[t].reachPile.size() - 1];
                    if (!q.empty())
                        q.pop();

                    while (!q.empty()) {

                        t = q.front();      // t��
                        q.pop();
                        if (!vis2[t]) {

                            for (int j = head[t]; j != -1; j = edge[j].next) {
                                int newT = edge[j].to;
                                if (vis2[newT] && newT != bus.start) {
                                    if (minDist2start[newT] < curMinDist) {
                                        tmpOKPath[newT] = j;
                                        curMinDist = minDist2start[newT];
                                        firstOKPoint = newT;
                                        OKPile = node[newT].reachPile[node[newT].reachPile.size() - 1];
                                    }
                                }
                            }
                            continue;
                        }
                            
                        if (t == bus.start)
                            break;
                        
                        if (minDist2start[t] < curMinDist) {
                            curMinDist = minDist2start[t];
                            firstOKPoint = t;
                            OKPile = node[t].reachPile[node[t].reachPile.size() - 1];
                        }

                    }

                }

                getOutFlag = true;
                break;
            }
            else {  // ˵��t��Ҳ�ǲ��ɴ��
                q.push(t);
            }

        }
    }

    dijkstra4(firstOKPoint, bus.start, OKPile, tmpOKPath);  // tmpOKPath�洢��·���Ǵ��յ㵽����·����ʹ��ʱҪע��

    int curNode = bus.start;
    bus.pileId = OKPile;
    bool addEdgeFlag = false;
    while (tmpOKPath[curNode] != -1) {
        int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id

        if (curNode == firstOKPoint) {
            addEdgeFlag = true;
        }

        if (addEdgeFlag) {
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);

            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);

            bus.path.push_back((cntEdge - 1) / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
            edge[(cntEdge - 1)].Pile[OKPile] = bus.busId;

            if ((cntEdge - 1) % 2) // ����-1
                edge[(cntEdge - 1) - 1].Pile[OKPile] = bus.busId;   // ˫��ߣ�����һ����
            else            // ż��+1
                edge[(cntEdge - 1) + 1].Pile[OKPile] = bus.busId;
        }

        if (!addEdgeFlag) {
            bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
            edge[edgeId].Pile[OKPile] = bus.busId;

            if (edgeId % 2) // ����-1
                edge[edgeId - 1].Pile[OKPile] = bus.busId;   // ˫��ߣ�����һ����
            else            // ż��+1
                edge[edgeId + 1].Pile[OKPile] = bus.busId;
        }

        curNode = edge[edgeId].from;

    }
}

// ����ͨ����������Ӳ���ҵ��ʱ��Ѱ�Һ��ʵ�λ�ý��мӱ߲��������ٴ�Ѱ·
void findAddPath2(Business& bus, int* dist2start) {

    int end = bus.end;  // end���ڴ�֮ǰ��֤���ɴ�
    vector<bool> vis3(N, false);
    vis3[end] = true;
    queue<int> q;
    q.push(end);
    int firstOKPoint = bus.start;
    int OKPile = -1;
    vector<int> tmpOKPath(N, -1);   // �洢·���������ǵ�ÿһ�����ǰһ���ߵı��

    bool getOutFlag = false;
    while (!q.empty() && !getOutFlag) {    // Ѱ��end����һ���ɴ���·��

        int curNode = q.front();
        q.pop();
        for (int i = head[curNode]; i != -1; i = edge[i].next) {

            int t = edge[i].to;
            if (vis3[t])
                continue;
            vis3[t] = true;
            tmpOKPath[t] = i;

            if (dist2start[t] >= 0) {

                firstOKPoint = t;
                if (t == bus.start) {
                    OKPile = 0;
                    getOutFlag = true;
                    break;
                }
                else {

                    // һ���������п��е㣬���ٽ���q.push����
                    // ���������µĶ����Ѱ�Ҿ�����������

                    OKPile = node[t].reachPile[node[t].reachPile.size() - 1];
                    getOutFlag = true;
                    break;
                }

            }
            else {  // ˵��t��Ҳ�ǲ��ɴ��
                q.push(t);
            }

        }
    }

    BFS4(firstOKPoint, bus.start, OKPile, tmpOKPath);  // tmpOKPath�洢��·���Ǵ��յ㵽����·����ʹ��ʱҪע��

    int curNode = bus.start;
    bus.pileId = OKPile;
    bool addEdgeFlag = false;
    while (tmpOKPath[curNode] != -1) {
        int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id

        if (curNode == firstOKPoint) {
            addEdgeFlag = true;
        }

        if (addEdgeFlag) {
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);

            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);

            bus.path.push_back((cntEdge - 1) / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
            edge[(cntEdge - 1)].Pile[OKPile] = bus.busId;

            if ((cntEdge - 1) % 2) // ����-1
                edge[(cntEdge - 1) - 1].Pile[OKPile] = bus.busId;   // ˫��ߣ�����һ����
            else            // ż��+1
                edge[(cntEdge - 1) + 1].Pile[OKPile] = bus.busId;
        }

        if (!addEdgeFlag) {
            bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
            edge[edgeId].Pile[OKPile] = bus.busId;

            if (edgeId % 2) // ����-1
                edge[edgeId - 1].Pile[OKPile] = bus.busId;   // ˫��ߣ�����һ����
            else            // ż��+1
                edge[edgeId + 1].Pile[OKPile] = bus.busId;
        }

        curNode = edge[edgeId].from;

    }
}
