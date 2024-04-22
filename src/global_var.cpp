#include "global_var.h"

int N, M, T, P, D = 0;
int cntEdge = 0;
int cntBus = 0;

int head[Configure::maxN];
bool vis[Configure::maxN];
std::vector<std::pair<int, int>> newEdge;
std::vector<int> newEdgePathId;
std::vector<int> remainBus;
Node node[Configure::maxN];
Edge edge[Configure::maxM];
Business buses[Configure::maxBus];
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;
