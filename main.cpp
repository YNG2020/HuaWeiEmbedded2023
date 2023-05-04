#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <math.h>
#include <time.h>
const int INF = 2147483647;

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
    int usedPileCnt;
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
        pileId = -1;
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
vector<pair<int, int>> newEdge; // ��¼����ӵıߵ������յ�
vector<int> newEdgePathId;   // ��¼�±��ڱ߼��е�λ�ã�����ʱ��˫�����Ϊͬһ�ߣ�
vector<int> remainBus;  // ��¼�³��η���ʱ����·���������޷�����ߵ�ҵ��ı��

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
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;   // ��¼�����ڵ������·������

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

class Node2 {
public:
    int nodeId; // �ڵ���
    int curLevel;   // �ڵ㵽Դ��ľ���
    int usedPileCnt;    // Դ�㵽�ڵ��·���еı����Ѿ�ʹ���˵�ͨ�����ܺ�
    Node2(int nodeId, int curLevel, int usedPileCnt) {
        this->nodeId = nodeId;
        this->curLevel = curLevel;
        this->usedPileCnt = usedPileCnt;
    }
    Node2() {}
    bool operator < (const Node2& x) const { // ���������������Ԫ�ط��ڶѶ�
        return usedPileCnt < x.usedPileCnt;
    }
};

class Business1 {
public:
    int start;  // ҵ�����
    int end;    // ҵ���յ�
    int busId;  // ҵ��Id

    Business1(int start, int end, int busId) {
        this->start = start;
        this->end = end;
        this->busId = busId;
    }

    Business1() {}
    bool operator < (const Business1& a) const {
        if (a.start == start)
            return end > a.end;
        return start > a.start;
    }
};

priority_queue<Node1>q;  // ���ȶ��У��������ʱ�ĵ�nodeId��Դ��ľ����nodeId��ű���
priority_queue<Business1>busUndirectedQueue;  //ҵ��ȥ�򻯶���
priority_queue<Business1>busDirectedQueue;  //ҵ���������

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void dijkstra1(Business& bus);
void dijkstra2(Business& bus);
void dijkstra3(Business& bus);
void dijkstra4(int start, int end, int pileId, vector<int>& tmpOKPath);
bool dijkstra5(Business& bus, int blockEdge);
void dijkstra7(Business& bus);
void BFS1(Business& bus, bool ifLoadNewEdge);
void BFS2(Business& bus);
bool BFS5(Business& bus, int blockEdge);
bool BFS6(Business& bus, int blockEdge);
void BFS7(Business& bus);
void BFS9(Business& bus);
void loadBus(int busId, bool ifLoadRemain);
void allocateBus();
void createBusQueue();
void reAllocateBus(int HLim);
void tryDeleteEdge();
void reverseArray(vector<int>& arr);
void outPut();
bool bfsTestConnection(int start, int end);
void findAddPath(Business& bus, bool* vis2);
void reCoverNetwork(int lastBusID, int lastPileId);
void reloadBus(int lastBusID, int lastPileId, vector<int>& pathTmp);

bool ifLast = false;
bool ifTryDeleteEdge = true;

// ������
int main() {

    clock_t startTime, curTime, reAllocateTime, tryDeleteTime;
    double reAllocateUnitTime = 0, tryDeleteUnitTime = 0;
    startTime = clock();
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
    curTime = clock();

    double leftTime = 120 - double(curTime - startTime) / CLOCKS_PER_SEC;

    if (T > 4000 || T <= 3500) {
        while (leftTime > 3 * (tryDeleteUnitTime + reAllocateUnitTime)) {

            reAllocateTime = clock();
            reAllocateBus(T);
            curTime = clock();
            reAllocateUnitTime = double(curTime - reAllocateTime) / CLOCKS_PER_SEC;

            tryDeleteTime = clock();
            tryDeleteEdge();
            curTime = clock();
            tryDeleteUnitTime = double(curTime - tryDeleteTime) / CLOCKS_PER_SEC;

            leftTime = 120 - double(curTime - startTime) / CLOCKS_PER_SEC;
        }
    }

        
    ifLast = true;
    if (T > 4000 || T <= 3500) {
        tryDeleteEdge();
    }
    else {
        tryDeleteEdge();
        tryDeleteEdge();
    }
    outPut();

    return 0;
}

// �����е�ҵ����䵽��������
void allocateBus() {
    Business1 temp, next;
    vector<Business1> remainBus;
    bool isSame = false, isExist1 = false, isExist2 = false;
    createBusQueue();  //���ɶ���
    //��ȥ�򻯶������ҵ�����յ���ͬ���䷴���ҵ������ص�������
    while (!busUndirectedQueue.empty()) {
        temp = busUndirectedQueue.top();
        busUndirectedQueue.pop();
        if (!busUndirectedQueue.empty()) {
            next = busUndirectedQueue.top();
            //����һ��ͬ����յ�
            if (next.start == temp.start && next.end == temp.end) {
                isSame = true;
                loadBus(temp.busId, false);
            }
            //����һ����ͬ����յ�
            else {
                //�����һ������ͬ�����Ǹ���ͬ������һ��
                if (isSame) {
                    isSame = false;
                    loadBus(temp.busId, false);
                }
            }
        }
        //�����һ����
        else {
            if (isSame) {
                isSame = false;
                loadBus(temp.busId, false);
            }
        }
    }
    //������������ҵ������ͬ�յ㲻ͬ��ҵ����
    while (!busDirectedQueue.empty()) {
        isExist1 = false;
        isExist2 = false;
        //��δ�����ص�ҵ��
        while (!busDirectedQueue.empty()) {
            temp = busDirectedQueue.top();
            if (buses[temp.busId].pileId == -1) {
                isExist1 = true;
                break;
            }
            else
                busDirectedQueue.pop();
        }
        //����һ��δ�����ص�ҵ��
        while (!busDirectedQueue.empty()) {
            next = busDirectedQueue.top();
            if (buses[next.busId].pileId == -1) {
                isExist2 = true;
                break;
            }
            else
                busDirectedQueue.pop();
        }
        //������ҵ���
        if (isExist1 && isExist2) {
            //��������ͬ
            if (temp.start == next.start) {
                isSame = true;
                loadBus(temp.busId, false);
            }
            //�����㲻ͬ
            else {
                if (isSame) {
                    isSame = false;
                    loadBus(temp.busId, false);
                }
                else
                    remainBus.push_back(temp);
            }
        }
        //���ֻ�ҵ���temp
        else if (isExist1 && !isExist2) {
            if (isSame) {
                isSame = false;
                loadBus(temp.busId, false);
            }
            else
                remainBus.push_back(temp);
        }
    }
    //��ʣ��δ���ص�ҵ����ص������磨���������
    for (int i = 0; i < remainBus.size(); ++i)
        loadBus(remainBus[i].busId, false);
}

//����ҵ�����˳�����
void createBusQueue() {

    Business bus;
    for (int i = 0; i < T; ++i) {
        bus = buses[i];
        //ȥ�򻯣�ʹ����յ��෴��ҵ��Ҳ�ܹ��ൽһ��
        if (bus.start > bus.end) {
            busUndirectedQueue.push(Business1(bus.end, bus.start, bus.busId));
        }
        else {
            busUndirectedQueue.push(Business1(bus.start, bus.end, bus.busId));
        }
        busDirectedQueue.push(Business1(bus.start, bus.end, bus.busId));
    }
}

// ��ͼ���·���ҵ�񵽹������У���ʱ���ã�
void reAllocateBus(int HLim) {

    int gap = max(int(0.025 * T), 20);
    if (gap > T)
        return;
    vector<int> totBusIdx(T, 0);
    vector<int> busIdx(gap, 0);
    for (int i = 0; i < T; ++i)
        totBusIdx[i] = i;

    for (int i = 0; i + 19 < HLim; i = i + gap) {

        srand(time(NULL));  // �������������  
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

        if (1.025 * curEdgeNum < oriEdgeNum) {  // ����ı������٣�����Ǩ��
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

                findPath = BFS5(buses[lastBusIds[k]], idxEdge);
                //findPath = dijkstra5(buses[lastBusIds[k]], idxEdge);

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

                for (int k = 0; k < stopK; ++k) {   // ����ͼѰ·ʱ����ɵĶ������Ӱ������
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

    BFS1(buses[busId], false);
    //dijkstra1(buses[busId]);

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

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void dijkstra1(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;
    static int addNewEdgeCnt = 0;  // �ӱߴ��������Ǳ�����
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

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

            if (s == end) // ��end�Ѿ����뵽�������������������
                break;

            // û�б���������Ҫ����
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������

                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    int t = edge[i].to;

                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                        dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                        q.push(Node1(dis[t], t));   // ���±������ĵ�������
                    }
                }

            }
        }
        if (s == end) { // ��end�Ѿ����뵽�������������������

            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // ��������
                minPathSize[make_pair(start, end)] = dis[s];
            else if (minPathSize[make_pair(start, end)] > dis[s])
                minPathSize[make_pair(start, end)] = dis[s];

            if (dis[s] > 3 * minPathSize[make_pair(start, end)])  // �ҵ���·������̫������Ը��Ҫ
                continue;

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // �Ҳ���·����Ҫ�����±�
        if (T > 3500 && T <= 4000) {
            if (++addNewEdgeCnt % 2 == 0 && (bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                tryDeleteEdge();
        }
        else
            if ((bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                tryDeleteEdge();
        dijkstra7(bus);
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

    int start = bus.start, end = bus.end;

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

        if (s == end)   // ��end�Ѿ����뵽�������������������
            break;

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
        newEdgePathId.emplace_back(cntEdge / 2 - 1);

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

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������������Ҫ�ӱߵ��������������ֱ�ӷ���
bool dijkstra5(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

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

            if (s == end)   // ��end�Ѿ����뵽�������������������
                break;

            // û�б���������Ҫ����
            if (vis1[s])
                continue;

            vis1[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������

                if (i / 2 == blockEdge)
                    continue;

                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    int t = edge[i].to;
                    if (dis[t] > dis[s] + edge[i].d) {
                        tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                        dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                        q.push(Node1(dis[t], t));   // ���±������ĵ�������
                    }
                }

            }
        }
        if (s == end) { // ��end�Ѿ����뵽�������������������

            if (dis[s] > 3 * minPathSize[make_pair(start, end)])  // �ҵ���·������̫������Ը��Ҫ
                continue;

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
                if (curNode == start)
                    break;
            }
            if (tmpDist <= minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // �Ҳ���·��ֱ�ӷ���
        return false;
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
    return true;

}

// ���ܼ���ҵ��ʱ��Ѱ��ҵ��bus����㵽�յ��·����������ͨ��������ȫͨ��������������·���ϵķ��������ı߽��мӱߴ���
void dijkstra7(Business& bus) {

    int start = bus.start, end = bus.end;
    int minBlockEdge = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    tmpOKPath.resize(N, -1);
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

        if (s == end)   // ��end�Ѿ����뵽�������������������
            break;

        // û�б���������Ҫ����
        if (vis1[s])
            continue;
        vis1[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                q.push(Node1(dis[t], t));   // ���±������ĵ�������  
            }
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

            if (edgeId % 2) // ����-1
                edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
            else            // ż��+1
                edge[edgeId + 1].Pile[choosenP] = bus.busId;
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
            edge[cntEdge - 1].Pile[choosenP] = bus.busId;   // ż��+1
            bus.pathTmp[lastNode] = cntEdge - 2;
        }

    }
    reverseArray(bus.path);

}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������ٱ���·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void BFS1(Business& bus, bool ifLoadNewEdge) {

    int start = bus.start, end = bus.end, p = 0;
    static int addNewEdgeCnt = 0;  // �ӱߴ��������Ǳ�����
    static int addNewBus = 0;   // ��ҵ�����
    ++addNewBus;
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;
    double maxValue = -1;

    if (addNewBus % 2)
        p = 0;
    else
        p = P - 1;

    while (true) {
        if (addNewBus % 2) {
            ++p;
            if (p >= P)
                break;
        }
        else {
            --p;
            if (p < 0)
                break;
        }


    //if (T > 3500 && T <= 4000 && addNewBus % 4) {
    //    srand(time(NULL));  // �������������  
    //    random_shuffle(pileIdx.begin(), pileIdx.end());
    //}
    //else if (addNewBus % 2) {
    //    srand(time(NULL));  // �������������  
    //    random_shuffle(pileIdx.begin(), pileIdx.end());
    //}

    //for (int k = 0; k < P; ++k) {
    //    p = pileIdx[k];

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            vis1[i] = false;
        }
        queue<pair<int, int>> bfsQ;
        //queue<Node2> bfsq;
        bfsQ.push(make_pair(start, 0));
        //bfsq.push(Node2(start, 0, 0));
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        //int totUsedPileCnt = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) { // ����Ϊ�ռ������е㶼�����뵽��������ȥ��

            //Node2 tmpNode = bfsq.front();
            s = bfsQ.front().first;
            //s = tmpNode.nodeId;
            curLevel = bfsQ.front().second;
            //curLevel = tmpNode.curLevel;
            //totUsedPileCnt = tmpNode.usedPileCnt;
            bfsQ.pop();
            //bfsq.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

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
                        //totUsedPileCnt += edge[i].usedPileCnt;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                        //bfsq.push(Node2(t, curLevel + 1, totUsedPileCnt + edge[i].usedPileCnt));
                    }

                }
            }

        }
        if (s == end) {

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
            //double tmpValue = (0.5 * totUsedPileCnt + 0.0001) / curLevel;
            if (tmpDist < minPathDist) {
                //if (tmpValue >= maxValue) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;
        }

    }
    if (findPath == false) {    // �Ҳ���·����Ҫ�����±�
        //��ɾ��
        //if (T > 6000) {                 // �зֲ�
        //    if (++addNewEdgeCnt % 3 == 0)
        //        tryDeleteEdge();
        //}
        //else if (T > 5000 && T <= 6000) {
        //    if (++addNewEdgeCnt % 3 == 0)
        //        tryDeleteEdge();
        //}
        //else if (T > 4000 && T <= 5000) {   // �޷ֲ�
        //    if (++addNewEdgeCnt % 3 == 0)
        //        tryDeleteEdge();
        //}
        //else if (T > 3500 && T <= 4000) {   // ��һ���ر����������
        //    if (++addNewEdgeCnt % 5 == 0)
        //        tryDeleteEdge();
        //}
        //else if (T <= 3500) {
        //    if (++addNewEdgeCnt % 2 == 0)
        //        tryDeleteEdge();
        //}

        if (ifTryDeleteEdge) {
            if (T > 3500 && T <= 4000) {
                if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                    tryDeleteEdge();
            }
            else
                if (/*++addNewEdgeCnt % 2 == 0 && */(bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                    tryDeleteEdge();
        }

        //BFS2(bus);       // �ɵļӱ߲��ԣ�һ���ӱߣ�����·������ӣ���ȫ�������ǵ�ǰ��õ�
        BFS7(bus);       // �¼ӱ߲��ԣ�ֻ�����·������Ҫ���мӱߵı�
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
            int t = edge[i].to;
            if (vis1[t])
                continue;
            vis1[t] = true;
            bus.trueMinPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
            if (t == end) {
                getOutFlag = true;
                s = t;
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
        newEdgePathId.emplace_back(cntEdge / 2 - 1);

        curNode = edge[bus.trueMinPath[curNode]].from;
    }
    BFS1(bus, true);

}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������������Ҫ�ӱߵ��������������ֱ�ӷ���
bool BFS5(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {
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

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������������Ҫ�ӱߵ��������������ֱ�ӷ��أ��ҵ�·����Ҳֱ�ӷ��أ�������������
bool BFS6(Business& bus, int blockEdge) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {
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
                        return true;
                        ++curLevel;
                        break;
                    }
                    else
                        bfsQ.push(make_pair(t, curLevel + 1));

                }
            }

        }

    }

}

// ���ܼ���ҵ��ʱ��Ѱ��ҵ��bus����㵽�յ��·����������ͨ��������ȫͨ��������������·���ϵķ��������ı߽��мӱߴ���
void BFS7(Business& bus) {

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

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������ٱ���·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void BFS9(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;
    static int addNewEdgeCnt = 0;  // �ӱߴ��������Ǳ�����
    bool findPath = false;
    int minPathDist = INF;
    int choosenP = -1;
    vector<int> tmpOKPath;

    for (; p < P; ++p) {
        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            vis1[i] = false;
        }
        priority_queue<Node2>bfsq;
        queue<pair<int, int>> bfsQ;
        bfsQ.push(make_pair(start, 0));
        bfsq.emplace(start, 0, 0);
        vis1[start] = true;
        int s = start;
        int curLevel = 0;
        int totUsedPileCnt = 0;
        bool getOutFlag = false;

        while (!bfsQ.empty() && !getOutFlag) { // ����Ϊ�ռ������е㶼�����뵽��������ȥ��

            Node2 tmpNode = bfsq.top();
            s = bfsQ.front().first;
            s = tmpNode.nodeId;
            curLevel = bfsQ.front().second;
            curLevel = tmpNode.curLevel;
            totUsedPileCnt = tmpNode.usedPileCnt;
            bfsQ.pop();
            bfsq.pop();

            for (int i = head[s]; i != -1; i = edge[i].next) {

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
                        totUsedPileCnt += edge[i].usedPileCnt;
                        break;
                    }
                    else {
                        bfsQ.push(make_pair(t, curLevel + 1));
                        bfsq.emplace(t, curLevel + 1, totUsedPileCnt + edge[i].usedPileCnt);
                    }

                }
            }

        }
        if (s == end) {

            if (minPathSize.find(make_pair(start, end)) == minPathSize.end())   // ��������
                minPathSize[make_pair(start, end)] = curLevel;
            else if (minPathSize[make_pair(start, end)] > curLevel) {
                minPathSize[make_pair(start, end)] = curLevel;
            }

            if (curLevel > 3 * minPathSize[make_pair(start, end)])  // �ҵ���·������̫������Ը��Ҫ
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

        if (T > 3500 && T <= 4000) {
            if (++addNewEdgeCnt % 2 == 0 && (bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                tryDeleteEdge();
        }
        else
            if ((bus.start != buses[bus.busId - 1].start || bus.end != buses[bus.busId - 1].end))
                tryDeleteEdge();

        BFS7(bus);       // �¼ӱ߲��ԣ�ֻ�����·������Ҫ���мӱߵı�
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

        curNode = edge[bus.pathTmp[curNode]].from;
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
void findAddPath(Business& bus, bool* vis2) {

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
                else
                    OKPile = node[t].reachPile[0];
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
            newEdgePathId.emplace_back(cntEdge / 2 - 1);

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

    ////////////////////////////////////////////////////////////////////////
    // ��նԼӷŴ�����Ӱ��
    buses[lastBusID].curA = D;
    curNode = buses[lastPileId].start;
    int trueNextEdgeId;
    for (int i = 0; i < buses[lastPileId].path.size(); ++i) {

        if (edge[buses[lastPileId].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[lastPileId].path[i] * 2;
        else
            trueNextEdgeId = buses[lastPileId].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[lastPileId].curA >= edge[trueNextEdgeId].trueD) {
            buses[lastPileId].curA -= edge[trueNextEdgeId].trueD;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[lastPileId].pileId] = -1;
            buses[lastBusID].curA = D;
            buses[lastBusID].curA -= edge[trueNextEdgeId].trueD;
        }
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
    curNode = buses[lastBusID].start;
    int trueNextEdgeId;
    for (int i = 0; i < buses[lastBusID].path.size(); ++i) {

        if (edge[buses[lastBusID].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[lastBusID].path[i] * 2;
        else
            trueNextEdgeId = buses[lastBusID].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[lastBusID].curA >= edge[trueNextEdgeId].trueD) {
            buses[lastBusID].curA -= edge[trueNextEdgeId].trueD;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[lastBusID].pileId] = buses[lastBusID].pileId;
            buses[lastBusID].curA = D;
            buses[lastBusID].curA -= edge[trueNextEdgeId].trueD;
            buses[lastBusID].mutiplierId.push_back(edge[trueNextEdgeId].from);
        }
    }
}
