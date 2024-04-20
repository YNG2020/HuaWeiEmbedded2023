#include "global_var.h"

int N, M, T, P, D = 0;
int cntEdge = 0;
int cntBus = 0;

int head[configure::maxN];
bool vis1[configure::maxN];
std::vector<std::pair<int, int>> newEdge;
std::vector<int> newEdgePathId;
std::vector<int> remainBus;