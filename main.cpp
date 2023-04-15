#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#define INF 2147483647  // �����볣��

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
}node[maxN];

class Edge {
public:
    int from, to, d, next;    // ��㣬�յ㣬�ߵľ��룬ͬ������һ������edge�еı��
    int Pile[maxP]; // �ñ��ϴ��ڵ�ͨ������¼���ǵ�ǰ���ص�ҵ��ı�ţ�������ҵ��ʱֵΪ-1
    bool isFull;    // ��ʶ���ϵ�ͨ���Ƿ�ȫ����ռ��
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
    vector<int> pathTmp;   // �洢����㵽����������·����ĩ�ߵı��
    vector<int> path;   // �洢·���������ı�
    vector<int> mutiplierId;    // �洢�������ķŴ������ڽڵ�ı��
}buses[maxBus];

int head[maxN]; // head[i]����ʾ��iΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
int dis[maxN];  // dis[i]����ʾ��Դ�㵽i������
bool vis[maxN];  // ��ʶ�õ����ޱ����뵽��������
bool vis2[maxN]; // ��ʶ�õ����������ĳҵ��ʱ����·���������ʹ�
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

void init();
void addEdge(int s, int t, int d);
void addBus(int start, int end);
void dijkstra1(Business& bus);
void dijkstra2(Business& bus);
void loadBus(int busId);
void allocateBus();
void reverseArray(vector<int>& arr);
void outPut();
bool bfsTestConnection(int start, int end);

// ������
int main() {

    cin >> N >> M >> T >> P >> D;
    init();
    int s = 0, t = 0, d = 0;
    for (int i = 0; i < M; ++i) {     
        cin >> s >> t >> d;
        addEdge(s, t, d);
        addEdge(t, s, d);   // ���˫���
    }



    int Sj, Tj;
    for (int i = 0; i < T; ++i) {
        cin >> Sj >> Tj;
        addBus(Sj, Tj); // ���ҵ��
    }

    allocateBus();
    outPut();

    return 0;
}

// �����е�ҵ����䵽��������
void allocateBus() {
    for (int i = 0; i < T; ++i) {
        if (buses[i].start == buses[i].end)
            continue;
        loadBus(i);
    }
}

// ��ҵ��busId���ص���������
void loadBus(int busId) {
    dijkstra1(buses[busId]);
    int curNode = buses[busId].start, trueNextEdgeId;
    for (int i = 0; i < buses[busId].path.size(); ++i) {
        
        if (edge[buses[busId].path[i] * 2].from == curNode)
            trueNextEdgeId = buses[busId].path[i] * 2;
        else
            trueNextEdgeId = buses[busId].path[i] * 2 + 1;
        curNode = edge[trueNextEdgeId].to;

        if (buses[busId].curA >= edge[trueNextEdgeId].d) {
            buses[busId].curA -= edge[trueNextEdgeId].d;
        }
        else {
            node[edge[trueNextEdgeId].from].Multiplier[buses[busId].pileId] = buses[busId].pileId;
            buses[busId].curA = D;
            buses[busId].curA -= edge[trueNextEdgeId].d;
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

        for (int j = 0; j < N; ++j)
            minDist[make_pair(i, j)] = INF;
    }

    for (int i = 0; i < T; ++i) {
        buses[i].curA = D;
    }

}

// �ӱߺ�����s��㣬t�յ㣬d����
void addEdge(int s, int t, int d) {
    edge[cntEdge].from = s; // ���
    edge[cntEdge].to = t;   // �յ�
    edge[cntEdge].d = d;    // ����
    edge[cntEdge].next = head[s];   // ��ʽǰ����sΪ�����һ���ߵı�ţ�head[s]������ǵ�ǰ��sΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
    edge[cntEdge].isFull = false;
    for (int i = 0; i < P; ++i)
        edge[cntEdge].Pile[i] = -1;

    head[s] = cntEdge++;    // ������sΪ�������߼��ϵĵ�һ�����ڱ߼������λ�ã���ţ�
    if (d < minDist[make_pair(s, t)])
        minDist[make_pair(s, t)] = d;
}

// ��ҵ����
void addBus(int start, int end) {
    buses[cntBus].start = start;
    buses[cntBus].end = end;
    buses[cntBus].busId = cntBus;
    ++cntBus;
}

// ����һ�߶�ͨ��������£�Ѱ��ҵ��bus����㵽�յ��·������һ�������·������Ϊ�п��ܱߵ�ͨ������ȫռ�ã�
void dijkstra1(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    bool findPath = false;
    vector<int> tmpOKPath;
    int minPathDist = INF;
    int choosenP = -1;

    for (int i = 0; i < N; ++i) {
        vis2[i] = false;
    }

    for (; p < P; ++p) {

        tmpOKPath.resize(N, -1);
        for (int i = 0; i < N; ++i) { // ����ֵ
            dis[i] = INF;
            vis[i] = false;
        }
        dis[start] = 0;  // Դ�㵽Դ��ľ���Ϊ0
        vis2[start] = true;
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
            if (vis[s])
                continue;
            
            vis[s] = true;
            for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������
                
                if (edge[i].Pile[p] == -1) {        // pileδ��ռ��ʱ������ͼ�߸ñ�
                    
                    int t = edge[i].to;
                    if (dis[t] > dis[s] + edge[i].d) {
                        vis2[t] = true;
                        tmpOKPath[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                        dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                        q.push(Node1(dis[t], t));   // ���±������ĵ�������
                        
                    }
                }

            }
        }
        if (s == end) {

            int curNode = end, tmpDist = 0;
            while (tmpOKPath[curNode] != -1) {
                int edgeId = tmpOKPath[curNode];  // �洢��edge�����������ıߵ�Id
                curNode = edge[edgeId].from;
                tmpDist += edge[edgeId].d;
            }
            if (tmpDist < minPathDist) {
                minPathDist = tmpDist;
                bus.pathTmp = vector<int>(tmpOKPath.begin(), tmpOKPath.end());
                choosenP = p;
            }
            findPath = true;

        }
    }

    if (findPath == false) {    // �Ҳ���·����Ҫ�����±�

        for (int i = 0; i < N; ++i) {
            if (vis2[i] == false) { // P��dijkstra���ִﲻ�˸õ�
                if (bfsTestConnection(i, end)) {
                    for (int j = head[i]; j != -1; j = edge[j].next) {
                        int t = edge[j].to;
                        if (bfsTestConnection(start, t)) {
                            addEdge(t, i, minDist[make_pair(i, t)]);
                            addEdge(i, t, minDist[make_pair(t, i)]);

                            if (i < t)
                                newEdge.emplace_back(i, t);
                            else
                                newEdge.emplace_back(t, i);

                            dijkstra1(bus);     // ÿ���һ�αߣ����̽���һ�����·������
                            return;
                        }
                    }
                }
            } 
        }
        return;
    }

    int curNode = end;
    bus.pileId = choosenP;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id
        
        bus.path.push_back(edgeId / 2); // edgeId / 2��Ϊ����Ӧ��ĿҪ��
        edge[edgeId].Pile[choosenP] = bus.busId;
        int i = 0;
        for (; i < P; ++i) {
            if (edge[edgeId].Pile[i] == -1)
                break;
        }
        if (i == P) {   // ���ҵ��󣬿���û�б�����ͨ���ı�
            edge[edgeId].isFull = true;
            if (edgeId % 2) // ����-1
                edge[edgeId - 1].isFull = true;   // ˫��ߣ�����һ����
            else            // ż��+1
                edge[edgeId + 1].isFull = true;
        }

        if (edgeId % 2) // ����-1
            edge[edgeId - 1].Pile[choosenP] = bus.busId;   // ˫��ߣ�����һ����
        else            // ż��+1
            edge[edgeId + 1].Pile[choosenP] = bus.busId;


        curNode = edge[edgeId].from;
    }
    reverseArray(bus.path);
}

void dijkstra2(Business& bus) {

    int start = bus.start, end = bus.end, p = 0;

    bus.pathTmp.resize(N, -1);
    for (int i = 0; i < N; ++i) { // ����ֵ
        dis[i] = INF;
        vis[i] = false;
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
        if (vis[s])
            continue; 
        vis[s] = true;
        for (int i = head[s]; i != -1; i = edge[i].next) { // �����Ѷ���������
            int t = edge[i].to;
            if (dis[t] > dis[s] + edge[i].d) {
                bus.pathTmp[t] = i;    // ��¼�µִ�·����t�ıߵı��i
                dis[t] = dis[s] + edge[i].d;   // �ɳڲ���
                q.push(Node1(dis[t], t));   // ���±������ĵ�������  
            }
        }
    }

    int curNode = end;
    while (bus.pathTmp[curNode] != -1) {
        int edgeId = bus.pathTmp[curNode];  // �洢��edge�����������ıߵ�Id
        if (edge[edgeId].isFull) {  // ���·���к���ͨ���������ıߣ����ڸñ߶�Ӧ�ĵ��������±�
            addEdge(edge[edgeId].from, edge[edgeId].to, minDist[make_pair(edge[edgeId].from, edge[edgeId].to)]);
            addEdge(edge[edgeId].to, edge[edgeId].from, minDist[make_pair(edge[edgeId].to, edge[edgeId].from)]);
            dijkstra1(bus);     // ÿ���һ�αߣ����̽���һ�����·������
            if (edge[edgeId].from < edge[edgeId].to)
                newEdge.emplace_back(edge[edgeId].from, edge[edgeId].to);
            else
                newEdge.emplace_back(edge[edgeId].to, edge[edgeId].from);
            return;
        }
        curNode = edge[bus.pathTmp[curNode]].from;
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
}
