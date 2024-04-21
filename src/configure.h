#pragma once

class Configure {
public:

	// 规模限制参数
	static const int maxM = 50000; // 边的最大数目
	static const int maxN = 5000;  // 节点的最大数目
	static const int maxBus = 10000;   // 业务的最大数目
	static const int maxP = 80;    // 最大单边通道数量P

	// 迭代参数
	static const int cntLimit = 20;	// 最大迭代次数

	// 调试参数
	static const bool forIterOutput = true;		// 是否在每次迭代过程中输出新增的边的总数
	static const bool forJudger = false;		// 是否要被判题器检测

	static const int INF = 2147483647;

};
