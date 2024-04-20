#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H
#include <vector>
#include <map>
#include "Configure.h"

extern int N, M, T, P, D;	// 节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D
extern int cntEdge;		// 当前边集数组所存储的边的数目
extern int cntBus;			// 当前业务数组所存储业务的数目

extern int head[Configure::maxN];		// head[i]，表示以i为起点的在逻辑上的第一条边在边集数组的位置（编号）
extern bool vis1[Configure::maxN];		// 标识该点有无被访问过
extern std::vector<std::pair<int, int>> newEdge; // 记录新添加的边的起点和终点
extern std::vector<int> newEdgePathId;		// 记录新边在边集中的位置（计数时，双向边视为同一边）
extern std::vector<int> remainBus;			// 记录下初次分配时，因路径堵塞而无法分配边的业务的编号

#endif    