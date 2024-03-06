#pragma once
#include <vector>
#include "configure.h"
#include <unordered_map>

using namespace std;
class Node {
public:
    int NodeId;           // ʵ��������������±�Ϳ���Ψһ��ʶNode����������һ������
    int Multiplier[configure::maxP]; // �ýڵ��ϴ��ڵķŴ�������¼���ǵ�ǰҪ�Ŵ��ͨ���ı�ţ��Ŵ���������ʱֵΪ-1
    vector<int> reachPile;  // �ڵ���dijkstra�����У��ɵִ�ö����pile���
};

class Edge {
public:
    int from;   // ���
    int to;     // �յ�
    int d;      // �ߵĳ��ȣ���1ʱ���ɱ�����dijkstra����ʱ�൱��BFS��
    int next;   // ͬ������һ������edge�еı��
    int trueD;  // �ߵ��������ȣ����ڼ���ߵ���ģ�����ӷŴ���
    int Pile[configure::maxP]; // �ñ��ϴ��ڵ�ͨ������¼���ǵ�ǰ���ص�ҵ��ı�ţ�������ҵ��ʱֵΪ-1
    int usedPileCnt;
    Edge() {
        from = -1;
        to = -1;
        d = 0;
        next = -1;
    }

};    // �߼�����

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
};

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

extern Node node[configure::maxN]; // ʹ��configure.h�ж����maxN
extern Edge edge[configure::maxM]; // ʹ��configure.h�ж����maxM
extern Business buses[configure::maxBus]; // ʹ��configure.h�ж����maxBus

extern unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;   // ��¼�����ڵ�����̱ߵĳ���
extern unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;   // ��¼�����ڵ������·������