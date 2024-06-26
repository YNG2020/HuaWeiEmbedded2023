#include "global_var.h"
#include <string>

int N, M, T, P, D = 0;
int cntEdge = 0;
int oriCntEdge = 0;
int cntTran = 0;
int totCost = 0;
int totUsedPile = 0;
int minTotUsedPile = 0;
bool ifIterSuccess = true;
string dataGenSeed;

int tail[Configure::maxN];
int head[Configure::maxN];
bool vis[Configure::maxN];
std::vector<std::pair<int, int>> newEdge;
std::vector<int> newEdgePathID;
std::vector<int> remainTran;
Node node[Configure::maxN];
Edge edge[Configure::maxM];
Transaction trans[Configure::maxTran];
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;
vector<int> sortedTranIndices;
int oriHead[Configure::maxN];
int oriTail[Configure::maxN];
Edge oriEdge[Configure::maxM];
unordered_map<pair<int, int>, std::vector<int>, HashFunc_t, Equalfunc_t> multiEdgeID;
unordered_map<pair<int, int>, std::vector<int>, HashFunc_t, Equalfunc_t> oriMultiEdgeID;
vector<bool> recordIterSuccess;
vector<int> recordIterNewEdgeNum;
vector<int> recordIterTotUsedPile;
vector<int> reallocatedTranSta;