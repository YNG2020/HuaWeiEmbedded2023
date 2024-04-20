#include "global_struct.h"

Node node[Configure::maxN];
Edge edge[Configure::maxM];
Business buses[Configure::maxBus];
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;