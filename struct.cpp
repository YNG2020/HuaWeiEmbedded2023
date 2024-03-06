#include "struct.h"

Node node[configure::maxN];
Edge edge[configure::maxM];
Business buses[configure::maxBus];
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minDist;
unordered_map<pair<int, int>, int, HashFunc_t, Equalfunc_t> minPathSize;